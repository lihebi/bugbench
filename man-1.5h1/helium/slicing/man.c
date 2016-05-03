
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/stat.h>		/* for chmod */
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <locale.h>

#ifndef R_OK
#define R_OK 4
#endif

extern char *index (const char *, int);		/* not always in <string.h> */
extern char *rindex (const char *, int);	/* not always in <string.h> */

#include "defs.h"
#include "gripes.h"
#include "man.h"
#include "manfile.h"
#include "manpath.h"
#include "man-config.h"
#include "man-getopt.h"
#include "to_cat.h"
#include "util.h"
#include "glob.h"
#include "different.h"

char *progname;
char *pager;
char *colon_sep_section_list;
char *roff_directive;
char *dohp = 0;
int do_irix;
int apropos;
int whatis;
int nocats;			/* set by -c option: do not use cat page */
				/* this means that cat pages must not be used,
				   perhaps because the user knows they are
				   old or corrupt or so */
int can_use_cache;		/* output device is a tty, width 80 */
				/* this means that the result may be written
				   in /var/cache, and may be read from there */
int findall;
int print_where;
int one_per_line;
int do_troff;
int preformat;
int debug;
int fhs;
int fsstnd;

static char **section_list;

#ifdef DO_COMPRESS
int do_compress = 1;
#else
int do_compress = 0;
#endif

#define BUFSIZE 8192

/*
 * Try to determine the line length to use.
 * Preferences: 1. MANWIDTH, 2. ioctl, 3. COLUMNS, 4. 80
 *
 * joey, 950902
 */

#include <sys/ioctl.h>

int line_length = 80;
int ll = 0;

static void
get_line_length(void){
     char *cp;
     int width;

     if (preformat) {
	  line_length = 80;
	  return;
     }
     if ((cp = getenv ("MANWIDTH")) != NULL && (width = atoi(cp)) > 0) {
	  line_length = width;
	  return;
     }
     if ((cp = getenv ("COLUMNS")) != NULL && (width = atoi(cp)) > 0)
	  line_length = width;
     else
	  line_length = 80;
}

static int
setll(void) {
     return
	  (!do_troff && (line_length < 66 || line_length > 80)) ?
	  line_length*9/10 : 0;
}

#define VERY_LONG_PAGE	"1100i"

static char *
setpl(void) {
     char *pl;
     if (do_troff)
	  return NULL;
     if (preformat)
	  pl = VERY_LONG_PAGE;
     else
     if ((pl = getenv("MANPL")) == 0) {
	  if (isatty(0) && isatty(1))
	       pl = VERY_LONG_PAGE;
	  else
	       pl = "11i";		/* old troff default */
     }
     return pl;
}

static char *
is_section (char * name) {
     char **vs;

     if (isdigit (name[0]) && !isdigit (name[1]) && strlen(name) < 5)
	  return my_strdup (name);

     for (vs = section_list; *vs != NULL; vs++)
	  if (strcmp (*vs, name) == 0)
	       return my_strdup (name);

     return NULL;
}


static void
remove_file (char *file) {
     int i;

     i = unlink (file);

     if (debug) {
	  if (i)
	       perror(file);
	  else
	       gripe (UNLINKED, file);
     }
}

static void
remove_other_catfiles (char *catfile) {
     char *pathname;
     char *t;
     char **gf;
     int offset;

     pathname = my_strdup(catfile);
     t = rindex(pathname, '.');
     if (t == NULL || strcmp(t, getval("COMPRESS_EXT")))
	  return;
     offset = t - pathname;
     strcpy(t, "*");
     gf = glob_filename (pathname);

     if (gf != (char **) -1 && gf != NULL) {
	  for ( ; *gf; gf++) {
	       if (strlen (*gf) <= offset) {
		    if (strlen (*gf) == offset)  /* uncompressed version */
			 remove_file (*gf);
		    continue;
	       }

	       if (!strcmp (*gf + offset, getval("COMPRESS_EXT")))
		    continue;

	       if (get_expander (*gf) != NULL)
		    remove_file (*gf);
	  }
     }
}

/*
 * Simply display the preformatted page.
 */
static int
display_cat_file (char *file) {
     int found;

     if (preformat)
	  return 1;		/* nothing to do - preformat only */

     found = 0;

     if (access (file, R_OK) == 0 && different_cat_file(file)) {
	  char *command = NULL;
	  char *expander = get_expander (file);

	  if (expander != NULL && expander[0] != 0) {
	       if (isatty(1))
		    command = my_xsprintf("%s %s | %s", expander, file, pager);
	       else
		    command = my_xsprintf("%s %s", expander, file);
	  } else {
	       if (isatty(1)) {
		    command = my_xsprintf("%s %s", pager, file);
	       } else {
		    char *cat = getval("CAT");
		    command = my_xsprintf("%s %s", cat[0] ? cat : "cat", file);
	       }
	  }
	  found = !do_system_command (command, 0);
     }
     return found;
}

static char *
ultimate_source (char *name0) {
     FILE *fp;
     char *name;
     char *expander;
     int expfl = 0;
     char *fgr;
     char *beg;
     char *end;
     char *cp;
     char buf[BUFSIZE];
     static char ultname[BUFSIZE];

     name = my_strdup(name0);

again:
     expander = get_expander (name);
     if (expander && *expander) {
	  char *command;

	  command = my_xsprintf ("%s %s", expander, name);
	  fp = my_popen (command, "r");
	  if (fp == NULL) {
	       perror("popen");
	       gripe (EXPANSION_FAILED, command);
	       return (NULL);
	  }
	  fgr = fgets (buf, sizeof(buf), fp);
	  expfl = 1;
     } else {
	  fp = fopen (name, "r");
	  if (fp == NULL && expfl) {
	       char *extp = rindex (name0, '.');
	       if (extp && *extp) {
		    strcat(name, extp);		/* name==ultname here */
		    fp = fopen (name, "r");
	       }
	  }
	  else if (fp == NULL && get_expander(".gz")) {
	       strcat(name, ".gz");
	       fp = fopen (name, "r");
	  }

	  if (fp == NULL) {
	       gripe (OPEN_ERROR, name);
	       return (NULL);
	  }
	  fgr = fgets (buf, sizeof(buf), fp);
	  fclose (fp);
     }

     if (fgr == NULL) {
	  gripe (READ_ERROR, name);
	  return (NULL);
     }

     if (strncmp(buf, ".so", 3))
	  return (name);

     beg = buf+3;
     while (*beg == ' ' || *beg == '\t')
	  beg++;

     end = beg;
     while (*end != ' ' && *end != '\t' && *end != '\n' && *end != '\0')
	  end++;

     *end = '\0';

     if (name != ultname) {
	  strcpy(ultname, name);
	  name = ultname;
     }

     if ((cp = rindex(name, '/')) == NULL) /* very strange ... */
	  return 0;
     *cp = 0;
     if (!index(beg, '/')) {
	  strcat(name, "/");
	  strcat(name, beg);
     } else
     if((cp = rindex(name, '/')) != NULL && !strncmp(cp+1, "man", 3))
	  strcpy(cp+1, beg);
     else if((cp = rindex(beg, '/')) != NULL)
	  strcat(name, cp);
     else {
	  strcat(name, "/");
	  strcat(name, beg);
     }

     goto again;
}

static void
add_directive (char *d, char *file, char *buf, int buflen) {
     if ((d = getval(d)) != 0 && *d) {
	  if (*buf == 0) {
	       if (strlen(d) + strlen(file) + 2 > buflen)
		    return;
	       strcpy (buf, d);
	       strcat (buf, " ");
	       strcat (buf, file);
	  } else {
	       if (strlen(d) + strlen(buf) + 4 > buflen)
		    return;
	       strcat (buf, " | ");
	       strcat (buf, d);
	  }
     }
}

static int
parse_roff_directive (char *cp, char *file, char *buf, int buflen) {
     char c;
     int tbl_found = 0;

     while ((c = *cp++) != '\0') {
	  switch (c) {
	  case 'e':
	       if (debug)
		    gripe (FOUND_EQN);
	       add_directive ((do_troff ? "EQN" : "NEQN"), file, buf, buflen);
	       break;

	  case 'g':
	       if (debug)
		    gripe (FOUND_GRAP);
	       add_directive ("GRAP", file, buf, buflen);
	       break;

	  case 'p':
	       if (debug)
		    gripe (FOUND_PIC);
	       add_directive ("PIC", file, buf, buflen);
	       break;

	  case 't':
	       if (debug)
		    gripe (FOUND_TBL);
	       tbl_found++;
	       add_directive ("TBL", file, buf, buflen);
	       break;

	  case 'v':
	       if (debug)
		    gripe (FOUND_VGRIND);
	       add_directive ("VGRIND", file, buf, buflen);
	       break;

	  case 'r':
	       if (debug)
		    gripe (FOUND_REFER);
	       add_directive ("REFER", file, buf, buflen);
	       break;

	  case ' ':
	  case '\t':
	  case '\n':
	       goto done;

	  default:
	       return -1;
	  }
     }

done:
     if (*buf == 0)
	  return 1;

     add_directive (do_troff ? "TROFF" : "NROFF", "", buf, buflen);

     if (tbl_found && !do_troff && *getval("COL"))
	  add_directive ("COL", "", buf, buflen);

     return 0;
}

static char *
eos(char *s) {
     while(*s) s++;
     return s;
}

static char *
make_roff_command (char *file) {
     FILE *fp;
     static char buf [BUFSIZE];
     char line [BUFSIZE], bufh [BUFSIZE], buft [BUFSIZE];
     int status, ll;
     char *cp, *expander, *fgr, *pl;
     char *command = "";

     ll = setll();
     pl = setpl();

     expander = get_expander (file);

     bufh[0] = 0;
     if (ll || pl) {
	  strcat(bufh, "(echo -e \"");
	  if (ll)
	       sprintf(eos(bufh), ".ll %d.%di", ll/10, ll%10);
	  if (ll && pl)
	       sprintf(eos(bufh), "\\n");
	  if (pl)
	       sprintf(eos(bufh), ".pl %s", pl);
	  strcat(bufh, "\"; ");
     }

     buft[0] = 0;
     if (ll || pl) {
	  if (pl && !strcmp(pl, VERY_LONG_PAGE))
	      strcat(buft, "; echo \".pl \\n(nlu+10\"");
	  strcat(buft, ")");
     }

     if (expander && *expander) {
	  command = my_xsprintf("%s%s %s%s", bufh, expander, file, buft);
     } else if(ll || pl) {
	  char *cat = getval("CAT");

	  command = my_xsprintf("%s%s %s%s",
				bufh, cat[0] ? cat : "cat", file, buft);
     }

     if (strlen(command) >= sizeof(buf))
	  exit(1);
     strcpy(buf, command);

     if (roff_directive != NULL) {
	  if (debug)
	       gripe (ROFF_FROM_COMMAND_LINE);

	  status = parse_roff_directive (roff_directive, file,
					 buf, sizeof(buf));

	  if (status == 0)
	       return buf;

	  if (status == -1)
	       gripe (ROFF_CMD_FROM_COMMANDLINE_ERROR);
     }

     if (expander && *expander) {
	  char *cmd = my_xsprintf ("%s %s", expander, file);
	  fp = my_popen (cmd, "r");
	  if (fp == NULL) {
	       gripe (EXPANSION_FAILED, cmd);
	       return (NULL);
	  }
	  fgr = fgets (line, sizeof(line), fp);
     } else {
	  fp = fopen (file, "r");
	  if (fp == NULL) {
	       gripe (OPEN_ERROR, file);
	       return (NULL);
	  }
	  fgr = fgets (line, sizeof(line), fp);
	  fclose (fp);
     }

     if (fgr == NULL) {
	  gripe (READ_ERROR, file);
	  return (NULL);
     }

     cp = &line[0];
     if (*cp++ == '\'' && *cp++ == '\\' && *cp++ == '"' && *cp++ == ' ') {
	  if (debug)
	       gripe (ROFF_FROM_FILE, file);

	  status = parse_roff_directive (cp, file, buf, sizeof(buf));

	  if (status == 0)
	       return buf;

	  if (status == -1)
	       gripe (ROFF_CMD_FROM_FILE_ERROR, file);
     }

     if ((cp = getenv ("MANROFFSEQ")) != NULL) {
	  if (debug)
	       gripe (ROFF_FROM_ENV);

	  status = parse_roff_directive (cp, file, buf, sizeof(buf));

	  if (status == 0)
	       return buf;

	  if (status == -1)
	       gripe (MANROFFSEQ_ERROR);
     }

     if (debug)
	  gripe (USING_DEFAULT);

     (void) parse_roff_directive ("t", file, buf, sizeof(buf));

     return buf;
}

static int
make_cat_file (char *path, char *man_file, char *cat_file) {
     int mode;
     FILE *fp;
     char *roff_command;
     char *command = NULL;
     if ((fp = fopen (cat_file, "w")) == NULL) {
	  if (errno == ENOENT)		/* directory does not exist */
	       return 0;

	  if(unlink (cat_file) != 0 || (fp = fopen (cat_file, "w")) == NULL) {
	       if (errno == EROFS) 	/* possibly a CDROM */
		    return 0;
	       if (debug)
		    gripe (CAT_OPEN_ERROR, cat_file);
	       if (!suid)
		    return 0;

	       command = my_xsprintf("cp /dev/null %s 2>/dev/null", cat_file);
	       if (do_system_command(command, 1)) {
		    if (debug)
			 gripe (USER_CANNOT_OPEN_CAT);
		    return 0;
	       }
	       if (debug)
		    gripe (USER_CAN_OPEN_CAT);
	  }
     } else {
	  fclose (fp);
	  if (suid) {
	       if (chmod (cat_file, 0666)) {
		    if(unlink(cat_file) != 0) {
			 command = my_xsprintf("rm %s", cat_file);
			 (void) do_system_command (command, 1);
		    }
		    if ((fp = fopen (cat_file, "w")) != NULL)
			 fclose (fp);
	       }
          }
     }

     roff_command = make_roff_command (man_file);
     if (roff_command == NULL)
	  return 0;
     if (do_compress)
	  command = my_xsprintf("(cd %s ; %s | %s > %s)", path,
		   roff_command, getval("COMPRESS"), cat_file);
     else
	  command = my_xsprintf ("(cd %s ; %s > %s)", path,
		   roff_command, cat_file);
     signal (SIGINT, SIG_IGN);

     gripe (PLEASE_WAIT);

     if (!do_system_command (command, 0)) {
	  mode = ((ruid != euid) ? 0644 : (rgid != egid) ? 0464 : 0444);
	  if(chmod (cat_file, mode) != 0 && suid) {
	       command = my_xsprintf ("chmod 0%o %s", mode, cat_file);
	       (void) do_system_command (command, 1);
	  }
	  if (debug)
	       gripe (CHANGED_MODE, cat_file, mode);
     } else {
	  if(unlink(cat_file) != 0 && suid) {
	       command = my_xsprintf ("rm %s", cat_file);
	       (void) do_system_command (command, 1);
	  }
     }

     signal (SIGINT, SIG_DFL);

     return 1;
}

static int
display_man_file(char *path, char *man_file) {
     char *roff_command;
     char *command;

     if (!different_man_file (man_file))
	  return 0;
     roff_command = make_roff_command (man_file);
     if (roff_command == NULL)
	  return 0;
     if (do_troff)
	  command = my_xsprintf ("(cd %s ; %s)", path, roff_command);
     else
	  command = my_xsprintf ("(cd %s ; %s | %s)", path,
		   roff_command, pager);

     return !do_system_command (command, 0);
}

/*
 * make and display the cat file - return 0 if something went wrong
 */
static int
make_and_display_cat_file (char *path, char *man_file) {
     char *cat_file;
     char *ext;
     int status;
     int standards;

     ext = (do_compress ? getval("COMPRESS_EXT") : 0);
     standards = (fhs ? FHS : 0) | (fsstnd ? FSSTND : 0) | (dohp ? DO_HP : 0);

     if ((cat_file = convert_to_cat (man_file,ext,standards)) == NULL)
	  return 0;

     if (debug)
	  gripe (PROPOSED_CATFILE, cat_file);

     status = ((nocats | preformat) ? -2 : is_newer (man_file, cat_file));
     if (debug)
	  gripe (IS_NEWER_RESULT, status);
     if (status == -1 || status == -3) {
	  /* what? man_file does not exist anymore? */
	  gripe (CANNOT_STAT, man_file);
	  return(0);
     }

     if (status != 0 || access (cat_file, R_OK) != 0) {
	  if (print_where) {
	       return 1;
	  }

	  if (!make_cat_file (path, man_file, cat_file))
	       return 0;
	  if (status == -2 && do_compress)
	       remove_other_catfiles(cat_file);
     } else {
	  if (print_where) {
	       return 1;
	  }
     }
     (void) display_cat_file (cat_file);
     return 1;
}

static int
format_and_display (char *man_file) {
     char *path;

     if (access (man_file, R_OK) != 0)
	  return 0;

     path = mandir_of(man_file);
     if (path == NULL)
	  return 0;
     man_file = ultimate_source (man_file);
     if (man_file == NULL)
	  return 0;

     if (do_troff) {
	  char *command;
	  char *roff_command = make_roff_command (man_file);

	  if (roff_command == NULL)
	       return 0;

	  command = my_xsprintf("(cd %s ; %s)", path, roff_command);
	  return !do_system_command (command, 0);
     }
     if (can_use_cache && make_and_display_cat_file (path, man_file))
	  return 1;
     if (print_where) {
	  return 1;
     }

     return display_man_file (path, man_file);
}

static int
man (char *name, char *section) {
     int found, type, flags;
     struct manpage *mp;

     /* allow  man ./manpage  for formatting explicitly given man pages */
     if (index(name, '/')) {
	  char fullname[BUFSIZE];
	  char fullpath[BUFSIZE];
	  char *path;
	  char *cp;
	  FILE *fp = fopen(name, "r");

	  if (!fp) {
	       return 0;
	  }
	  fclose (fp);
	  if (*name != '/' && getcwd(fullname, sizeof(fullname))
	      && strlen(fullname) + strlen(name) + 3 < sizeof(fullname)) {
	       strcat (fullname, "/");
	       strcat (fullname, name);
	  } else if (strlen(name) + 2 < sizeof(fullname)) {
	       strcpy (fullname, name);
	  } else {
	       return 0;
	  }

	  strcpy (fullpath, fullname);
	  if ((cp = rindex(fullpath, '/')) != NULL
	      && cp-fullpath+4 < sizeof(fullpath)) {
	       strcpy(cp+1, "..");
	       path = fullpath;
	  } else
	       path = ".";

	  name = ultimate_source (fullname);
	  if (!name)
	       return 0;

	  if (print_where) {
	       return 1;
	  }
	  return display_man_file (path, name);
     }

     fflush (stdout);
     init_manpath();

     can_use_cache = (preformat || (isatty(0) && isatty(1) && !setll()));

     if (do_troff) {
	  char *t = getval("TROFF");
	  if (!t || !*t)
	       return 0;	/* don't know how to format */
	  type = TYPE_MAN;
     } else {
	  char *n = getval("NROFF");
	  type = 0;
	  if (can_use_cache)
	       type |= TYPE_CAT;
	  if (n && *n)
	       type |= TYPE_MAN;
	  if (fhs || fsstnd)
	       type |= TYPE_SCAT;
     }

     flags = type;
     if (!findall)
	  flags |= ONLY_ONE;
     if (fsstnd)
	  flags |= FSSTND;
     else if (fhs)
	  flags |= FHS;
     if (dohp)
	  flags |= DO_HP;
     if (do_irix)
	  flags |= DO_IRIX;

     mp = manfile(name, section, flags, section_list, mandirlist,
		  convert_to_cat);
     found = 0;
     while (mp) {
          if (mp->type == TYPE_MAN) {
	       found = format_and_display(mp->filename);
	  } else if (mp->type == TYPE_CAT || mp->type == TYPE_SCAT) {
               if (print_where) {
                    found = 1;
               } else
	            found = display_cat_file(mp->filename);
	  } else
	       break;
	  if (found && !findall)
	       break;
	  mp = mp->next;
     }
     return found;
}

static char **
get_section_list (void) {
     int i;
     char *p;
     char *end;
     static char *tmp_section_list[100];

     if (colon_sep_section_list == NULL) {
	  if ((p = getenv ("MANSECT")) == NULL)
	       p = getval ("MANSECT");
	  colon_sep_section_list = my_strdup (p);
     }

     i = 0;
     for (p = colon_sep_section_list; ; p = end+1) {
	  if ((end = strchr (p, ':')) != NULL)
	       *end = '\0';

	  tmp_section_list[i++] = my_strdup (p);

	  if (end == NULL || i+1 == sizeof(tmp_section_list))
	       break;
     }

     tmp_section_list [i] = NULL;
     return tmp_section_list;
}

static void
do_global_apropos (char *name, char *section) {
     char **dp, **gf;
     char *pathname;
     char *command;
     int res;

     init_manpath();
     if (mandirlist)
	for (dp = mandirlist; *dp; dp++) {
	  if (debug)
	       gripe(SEARCHING, *dp);
	  pathname = my_xsprintf("%s/man%s/*", *dp, section ? section : "*");
	  gf = glob_filename (pathname);

	  if (gf != (char **) -1 && gf != NULL) {
	       for( ; *gf; gf++) {
		    char *expander = get_expander (*gf);
		    if (expander)
			 command = my_xsprintf("%s %s | grep -%c '%s'",
				 expander, *gf, GREPSILENT, name);
		    else
			 command = my_xsprintf("grep -%c '%s' %s",
				 GREPSILENT, name, *gf);
		    res = do_system_command (command, 1);
		    if (res == 0) {
			 if (print_where)
			      printf("%s\n", *gf);
			 else {
			      int answer, c;
			      char path[BUFSIZE];

			      fflush(stdout);
			      answer = c = getchar();
			      while (c != '\n' && c != EOF)
				   c = getchar();
			      if(index("QqXx", answer))
				   exit(0);
			      if(index("YyJj", answer)) {
				   char *ri;

				   strcpy(path, *gf);
				   ri = rindex(path, '/');
				   if (ri)
					*ri = 0;
				   format_and_display(*gf);
			      }
			 }
		    }
	       }
	  }
     }
}

static void
do_apropos (char *name) {
     char *command;
     command = my_xsprintf("%s %s", getval("APROPOS"), name);
     (void) do_system_command (command, 0);
}

static void
do_whatis (char *name) {
     char *command;
     command = my_xsprintf("%s %s", getval("WHATIS"), name);
     (void) do_system_command (command, 0);
}

int
main (int argc, char **argv) {
     int status = 0;
     char *nextarg;
     char *tmp;
     char *section = 0;

#ifndef __FreeBSD__ 
     /* Slaven Rezif: FreeBSD-2.2-SNAP does not recognize LC_MESSAGES. */
     setlocale(LC_MESSAGES, "");
#endif
     dohp = getenv("MAN_HP_DIREXT");		/* .Z */
     if (getenv("MAN_IRIX_CATNAMES"))
	  do_irix = 1;
     progname = mkprogname (argv[0]);
     get_permissions ();
     get_line_length();
     man_getopt (argc, argv);
     if (!strcmp (progname, "manpath") || (optind == argc && print_where)) {
	  init_manpath();
	  exit(0);
     }

     if (optind == argc)
	  gripe(NO_NAME_NO_SECTION);

     section_list = get_section_list ();
     while (optind < argc) {
	  nextarg = argv[optind++];

	  tmp = is_section (nextarg);
	  if (tmp) {
	       if (optind < argc) {
		    section = tmp;
		    if (debug)
			 gripe (SECTION, section);
	       } else {
		    gripe (NO_NAME_FROM_SECTION, tmp);
	       }
	       continue;
	  }

	  if (global_apropos)
	       do_global_apropos (nextarg, section);
	  else if (apropos)
	       do_apropos (nextarg);
	  else if (whatis)
	       do_whatis (nextarg);
	  else {
	    //<------------segment fault within man~~~
	       status = man (nextarg, section);
	       if (status == 0) {
		    if (section)
			 gripe (NO_SUCH_ENTRY_IN_SECTION, nextarg, section);
		    else
			 gripe (NO_SUCH_ENTRY, nextarg);
	       }
	  }
     }
}
