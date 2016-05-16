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

#include <sys/ioctl.h>

int line_length = 80;
int ll = 0;

static int
man (char *name, char *section) {
     int found, type, flags;
     struct manpage *mp;

     found = 0;

     init_manpath();
     // ADDED BY HELIUM
     type = TYPE_MAN;
     flags = type;

     mp = manfile(name, section, flags, section_list, mandirlist,
		  convert_to_cat);

     found = 0;
     return found;
}

static char **
get_section_list (void) {
     int i;
     char *p;
     char *end;
     static char *tmp_section_list[100];
     i = 0;
     for (p = colon_sep_section_list; ; p = end+1) {
	  if ((end = strchr (p, ':')) != NULL)
	       *end = '\0';

	  tmp_section_list[i++] = my_strdup (p);

	  /* if (end == NULL || i+1 == sizeof(tmp_section_list)/sizeof(char*)) */
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
     char *section = 0;

     man_getopt (argc, argv);

     section_list = get_section_list ();
     
     while (optind < argc) {
	  nextarg = argv[optind++];
	    //<------------segment fault within man~~~
	  status = man (nextarg, section);
     }
     return !status;
}
