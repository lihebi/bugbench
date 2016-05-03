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
  // FINALLY this is the bug crash location!
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
     if (type == TYPE_CAT) {
          p = to_cat_filename(pathname, 0, standards);
     }
     names = glob_filename (pathname);
     return names;
}
static char **
glob_for_file_ext (char *dir, char *sec, char *name, char *ext, int type) {
     char **names, **namesglob;
     char *hpx = ((standards & DO_HP) ? ".Z" : "");

     namesglob = glob_for_file_ext_glob(dir,sec,name,ext,hpx,1,type);
     return namesglob;
}
static char **
glob_for_file (char *dir, char *sec, char *name, int type) {
     char **names;
     names = glob_for_file_ext (dir, sec, name, sec, type);
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
	  if (flags & type) {
	       names = glob_for_file (dir, sec, name, type);
	  }
     }
     return res;
}
static struct manpage *
manfile_from_section(char *name, char *section, int flags, char **manpath) {
     char **mp;
     struct manpage *res = 0;
     for (mp = manpath; *mp; mp++) {
	  append(&res, manfile_from_sec_and_dir(*mp, section, name, flags));
     }
     return res;
}
struct manpage *
manfile(char *name, char *section, int flags,
        char **sectionlist, char **manpath,
	char *((*tocat)(char *man_filename, char *ext, int flags))) {
     char **sl;
     struct manpage *res;
	  for (sl = sectionlist; *sl; sl++) {
	       append(&res, manfile_from_section(name, *sl, flags, manpath));
	  }
     return res;
}
