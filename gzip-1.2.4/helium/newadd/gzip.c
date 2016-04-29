#include <ctype.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>
#define local static
#include <stdio.h>
#if !defined(NO_STRING_H) || defined(STDC_HEADERS)
#  include <string.h>
#  if !defined(STDC_HEADERS) && !defined(NO_MEMORY_H) && !defined(__GNUC__)
#    include <memory.h>
#  endif
#  define memzero(s, n)     memset ((voidp)(s), 0, (n))
#else
#  include <strings.h>
#  define strchr            index 
#  define strrchr           rindex
#  define memcpy(d, s, n)   bcopy((s), (d), (n)) 
#  define memcmp(s1, s2, n) bcmp((s1), (s2), (n)) 
#  define memzero(s, n)     bzero((s), (n))
#endif

#  define OF(args)  args

#  define MAX_SUFFIX  30
char z_suffix[MAX_SUFFIX+1]; /* default suffix (can be set with --suffix) */
#  define Z_SUFFIX ".gz"


struct stat istat;         /* status for input file */
#define OK      0
#  define MAX_PATH_LEN   1024 /* max pathname length */

local void treat_file   OF((char *iname));
local int  get_istat    OF((char *iname, struct stat *sbuf));
      int main          OF((int argc, char **argv));
char ifname[MAX_PATH_LEN]; /* input file name */
int  z_len;           /* strlen(z_suffix) */
int optind = 0;
int exit_code = OK;   /* program exit code */







local void treat_file(iname)
    char *iname;
{
    /* Check if the input file is present, set ifname and istat: */
    if (get_istat(iname, &istat) != OK) return;
}


local int get_istat(iname, sbuf)
    char *iname;
    struct stat *sbuf;
{
    strcpy(ifname, iname);
}

int main (argc, argv)
    int argc;
    char **argv;
{
    int file_count;     /* number of files to precess */
    strncpy(z_suffix, Z_SUFFIX, sizeof(z_suffix)-1);
    z_len = strlen(z_suffix);
    file_count = argc - optind;
    /* And get to work */
    if (file_count != 0) {
        while (optind < argc) {
	    treat_file(argv[optind++]);
	}
    }
    return exit_code; /* just to avoid lint warning */
}

