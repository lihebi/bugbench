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

/*
 * Append the struct or chain A to the chain HEAD.
 */
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
  printf("my_lth, crash point!\n");
  // FINALLY this is the bug crash location!
     return s ? strlen(s) : 0;
}

/*
 * Find the files of the form DIR/manSEC/NAME.EXT etc.
 * Use "man" for TYPE_MAN, "cat" for TYPE_SCAT, and
 * apply convert_to_cat() to the man version for TYPE_CAT.
 *
 * Some HP systems use /usr/man/man1.Z/name.1, where name.1 is
 * compressed - yuk.  We can handle this by using section 1.Z
 * instead of 1 and assuming that the man page is compressed
 * if the directory name ends in .Z.
 *
 * Returns an array with pathnames, or 0 if out-of-memory or error.
 */
static char **
glob_for_file_ext_glob (char *dir, char *sec, char *name, char *ext, char *hpx,
			int glob, int type) {
  printf("entering glob_for_file_ext_glob\n");
     char *pathname;
     char *p;
     char **names;
     int len;

     len = my_lth(dir) + my_lth(sec) + my_lth(hpx) + my_lth(name)
	  + my_lth(ext) + 8;
     pathname = (char *) malloc(len);
     if (!pathname)
	  return 0;

     /* sprintf (pathname, "%s/%s%s%s/%s.%s%s", */
     /* 	      dir, (type == TYPE_SCAT) ? "cat" : "man", sec, hpx, */
     /* 	      name, ext, glob ? "*" : ""); */

     if (type == TYPE_CAT) {
          p = to_cat_filename(pathname, 0, standards);
          if (p) {
	       /* free(pathname); */
               /* pathname = p; */
          } else {
               sprintf (pathname, "%s/cat%s%s/%s.%s%s",
                        dir, sec, hpx, name, ext, glob ? "*" : "");
	  }
     }

     names = glob_filename (pathname);
     /* if (names == (char **) -1)		/\* file system error; print msg? *\/ */
     /* 	  names = 0; */
     return names;
}

static char **
glob_for_file_ext (char *dir, char *sec, char *name, char *ext, int type) {
  printf("enter glob_for_file_ext\n");
     char **names, **namesglob;
     char *hpx = ((standards & DO_HP) ? ".Z" : "");

     namesglob = glob_for_file_ext_glob(dir,sec,name,ext,hpx,1,type);
  printf("leave glob_for_file_ext\n");
     return namesglob;
}

/*
 * Find the files of the form DIR/manSEC/NAME.SEC etc.
 */
static char **
glob_for_file (char *dir, char *sec, char *name, int type) {
  printf("enter glob_for_file\n");
     char **names;
     names = glob_for_file_ext (dir, sec, name, sec, type);
     return names;
}

/*
 * Find a man page of the given NAME under the directory DIR,
 * in section SEC. Only types (man, cat, scat) permitted in FLAGS
 * are allowed, and priorities are in this order.
 */
static struct manpage *
manfile_from_sec_and_dir(char *dir, char *sec, char *name, int flags) {
     struct manpage *res = 0;
     struct manpage *p;
     char **names, **np;
     int types[3] = { TYPE_MAN, TYPE_CAT, TYPE_SCAT };
     int i, type;

	  printf("manfile from sec and dir\n");
     for (i=0; i<3; i++) {
	  type = types[i];

	  if (flags & type) {
	       names = glob_for_file (dir, sec, name, type);
	  }
     }
	  printf("end man from sec and dir\n");

     return res;
}

/*
 * Find a man page of the given NAME, searching in the specified SECTION.
 * Searching is done in all directories of MANPATH.
 */
static struct manpage *
manfile_from_section(char *name, char *section, int flags, char **manpath) {
     char **mp;
     struct manpage *res = 0;
     printf("manfile_from_section\n");
     for (mp = manpath; *mp; mp++) {
	  append(&res, manfile_from_sec_and_dir(*mp, section, name, flags));
     }
     printf("return of manfile from secton\n");

     return res;
}

/*
 * Find a man page of the given NAME, searching in the specified
 * SECTION, or, if that is 0, in all sections in SECTIONLIST.
 * Searching is done in all directories of MANPATH.
 * If FLAGS contains the ONLY_ONE bits, only the first matching
 * page is returned; otherwise all matching pages are found.
 * Only types (man, cat, scat) permitted in FLAGS are allowed.
 */
struct manpage *
manfile(char *name, char *section, int flags,
        char **sectionlist, char **manpath,
	char *((*tocat)(char *man_filename, char *ext, int flags))) {
     char **sl;
     struct manpage *res;
     	  printf("inside manpage\n");


     /* standards = (flags & (FHS | FSSTND | DO_HP | DO_IRIX)); */
     /* to_cat_filename = tocat; */

	  for (sl = sectionlist; *sl; sl++) {
	       append(&res, manfile_from_section(name, *sl, flags, manpath));
	  }
     return res;
}
