/*
 * manfile.c - aeb, 971231
 *
 * Used both by man and man2html - be careful with printing!
 */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "glob.h"
#include "manfile.h"

static int standards;
static char *((*to_cat_filename)(char *man_filename, char *ext, int flags));

static void
append(struct manpage **head, struct manpage *a) {
     struct manpage *p;

     if (a) {
	  if (*head) {
	       p = *head;
	       while(p->next)
		    p = p->next;
	       p->next = a;
	  } else
	       *head = a;
     }
}

static int
my_lth(char *s) {
     return s ? strlen(s) : 0;
}

static char **
glob_for_file_ext_glob (char *dir, char *sec, char *name, char *ext, char *hpx,
			int glob, int type) {
     char *pathname;
     char *p;
     char **names;
     int len;

     len = my_lth(dir) + my_lth(sec) + my_lth(hpx) + my_lth(name)
	  + my_lth(ext) + 8;
     pathname = (char *) malloc(len);
     if (!pathname)
	  return 0;

     sprintf (pathname, "%s/%s%s%s/%s.%s%s",
	      dir, (type == TYPE_SCAT) ? "cat" : "man", sec, hpx,
	      name, ext, glob ? "*" : "");

     if (type == TYPE_CAT) {
          p = to_cat_filename(pathname, 0, standards);
          if (p) {
               pathname = p;
          } else {
               sprintf (pathname, "%s/cat%s%s/%s.%s%s",
                        dir, sec, hpx, name, ext, glob ? "*" : "");
	  }
     }

     names = glob_filename (pathname);
     if (names == (char **) -1)		/* file system error; print msg? */
	  names = 0;
     return names;
}

static char **
glob_for_file_ext (char *dir, char *sec, char *name, char *ext, int type) {
     char **names, **namesglob;
     char *hpx = ((standards & DO_HP) ? ".Z" : "");

     namesglob = glob_for_file_ext_glob(dir,sec,name,ext,hpx,1,type);
     if (!namesglob && *hpx) {
	  hpx = "";
	  namesglob = glob_for_file_ext_glob(dir,sec,name,ext,hpx,1,type);
     }
     if (!namesglob)
	  return 0;
     if (*namesglob) {
	  names = glob_for_file_ext_glob(dir,sec,name,ext,hpx,0,type);
	  if (names && *names)
	       namesglob = names;
     }
     return namesglob;
}

static char **
glob_for_file (char *dir, char *sec, char *name, int type) {
     char **names;
     if (standards & DO_IRIX) {
	  return glob_for_file_ext (dir, sec, name, "", type);
     }
     names = glob_for_file_ext (dir, sec, name, sec, type);
     if (!names)
	  return 0;		/* out-of-memory or error */
     if (!*names && isdigit(sec[0]) && sec[1] != 0) {
	  char ext[2];
	  ext[0] = sec[0];
	  ext[1] = 0;
	  names = glob_for_file_ext (dir, sec, name, ext, type);
     }
     if (!names)
	  return 0;		/* out-of-memory or error */
     if (!*names)
	  names = glob_for_file_ext (dir, sec, name, "man", type);
     return names;
}

static struct manpage *
manfile_from_sec_and_dir(char *dir, char *sec, char *name, int flags) {
     struct manpage *res = 0;
     struct manpage *p;
     char **names, **np;
     int types[3] = { TYPE_MAN, TYPE_CAT, TYPE_SCAT };
     int i, type;

     for (i=0; i<3; i++) {
	  type = types[i];
	  if ((type == TYPE_CAT) && (flags & TYPE_SCAT) && !standards)
	       continue;

	  if (flags & type) {
	       names = glob_for_file (dir, sec, name, type);
	       if (names) {
		    for (np = names; *np; np++) {
			 p = (struct manpage *) malloc(sizeof(*p));
			 if (!p)
			      break; 	/* %% perhaps print msg, free names */
			 p->filename = *np;
			 p->type = type;
			 p->next = 0;
			 append(&res, p);
			 if (res && (flags & ONLY_ONE_PERSEC))
			      break;
		    }
	       }
	  }

	  if (res)
	       return res;
     }

     return res;
}

static struct manpage *
manfile_from_section(char *name, char *section, int flags, char **manpath) {
     char **mp;
     struct manpage *res = 0;

     for (mp = manpath; *mp; mp++) {
	  append(&res, manfile_from_sec_and_dir(*mp, section, name, flags));
	  if (res && (flags & ONLY_ONE_PERSEC))
	       break;
     }
     if (res == NULL && isdigit(section[0]) && section[1]) {
	  char sec[2];

	  sec[0] = section[0];
	  sec[1] = 0;
	  for (mp = manpath; *mp; mp++) {
	       append(&res, manfile_from_sec_and_dir(*mp, sec, name, flags));
	       if (res && (flags & ONLY_ONE_PERSEC))
		    break;
	  }
     }
     return res;
}

struct manpage *
manfile(char *name, char *section, int flags,
        char **sectionlist, char **manpath,
	char *((*tocat)(char *man_filename, char *ext, int flags))) {
     char **sl;
     struct manpage *res;

     standards = (flags & (FHS | FSSTND | DO_HP | DO_IRIX));
     to_cat_filename = tocat;

     if (!name || !manpath)	/* error msg? */
	  res = 0;
     else if (section)
	  res = manfile_from_section(name, section, flags, manpath);
     else if (sectionlist) {
	  res = 0;
	  for (sl = sectionlist; *sl; sl++) {
	       append(&res, manfile_from_section(name, *sl, flags, manpath));
	       if (res && (flags & ONLY_ONE))
		    break;
	  }
     }
     return res;
}
