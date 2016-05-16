#  define MAX_SUFFIX  30
char z_suffix[MAX_SUFFIX+1]; /* default suffix (can be set with --suffix) */
#  define Z_SUFFIX ".gz"


struct stat istat;         /* status for input file */
#define OK      0
#  define MAX_PATH_LEN   1024 /* max pathname length */
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
  while (optind < argc) {
    treat_file(argv[optind++]);
  }
}

