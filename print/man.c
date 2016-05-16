#define BUFSIZE 8192

static int
man (char *name, char *section) {
     int found, type, flags;
     struct manpage *mp;
     found = 0;
     if (index(name, '/')) {
	  char fullname[BUFSIZE];
	  char fullpath[BUFSIZE];
	  char *path;
	  char *cp;
	  FILE *fp = fopen(name, "r");
	  if (!fp) {
	       perror(name);
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
	       fprintf(stderr, "%s: name too long\n", name);
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
	       printf("%s\n", name);
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
                    printf ("%s\n", mp->filename);
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

int
main (int argc, char **argv) {
     int status = 0;
     char *nextarg;
     char *tmp;
     char *section = 0;
     dohp = getenv("MAN_HP_DIREXT");		/* .Z */
     if (getenv("MAN_IRIX_CATNAMES"))
	  do_irix = 1;
     progname = mkprogname (argv[0]);
     get_permissions ();
     get_line_length();
     man_getopt (argc, argv);
     if (!strcmp (progname, "manpath") || (optind == argc && print_where)) {
	  init_manpath();
	  prmanpath();
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
	       status = man (nextarg, section);
	       if (status == 0) {
		    if (section)
			 gripe (NO_SUCH_ENTRY_IN_SECTION, nextarg, section);
		    else
			 gripe (NO_SUCH_ENTRY, nextarg);
	       }
	  }
     }
     return !status;
}
