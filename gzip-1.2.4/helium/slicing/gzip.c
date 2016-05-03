#include <ctype.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>

#include "tailor.h"
#include "gzip.h"
#include "lzw.h"
#include "revision.h"
#include "getopt.h"

		/* configuration */

#if !defined(S_ISDIR) && defined(S_IFDIR)
#  define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif
#if !defined(S_ISREG) && defined(S_IFREG)
#  define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif

typedef RETSIGTYPE (*sig_type) OF((int));

#ifndef	O_BINARY
#  define  O_BINARY  0  /* creation mode for open() */
#endif

#ifndef O_CREAT
   /* Pure BSD system? */
#  include <sys/file.h>
#  ifndef O_CREAT
#    define O_CREAT FCREAT
#  endif
#  ifndef O_EXCL
#    define O_EXCL FEXCL
#  endif
#endif

#ifndef S_IRUSR
#  define S_IRUSR 0400
#endif
#ifndef S_IWUSR
#  define S_IWUSR 0200
#endif
#define RW_USER (S_IRUSR | S_IWUSR)  /* creation mode for open() */

#ifndef MAX_PATH_LEN
#  define MAX_PATH_LEN   1024 /* max pathname length */
#endif

#ifndef SEEK_END
#  define SEEK_END 2
#endif

#ifdef NO_OFF_T
  typedef long off_t;
  off_t lseek OF((int fd, off_t offset, int whence));
#endif

/* Separator for file name parts (see shorten_name()) */
#ifdef NO_MULTIPLE_DOTS
#  define PART_SEP "-"
#else
#  define PART_SEP "."
#endif

		/* global buffers */

DECLARE(uch, inbuf,  INBUFSIZ +INBUF_EXTRA);
DECLARE(uch, outbuf, OUTBUFSIZ+OUTBUF_EXTRA);
DECLARE(ush, d_buf,  DIST_BUFSIZE);
DECLARE(uch, window, 2L*WSIZE);
#ifndef MAXSEG_64K
    DECLARE(ush, tab_prefix, 1L<<BITS);
#else
#endif

int ascii = 0;        /* convert end-of-lines to local OS conventions */
int to_stdout = 0;    /* output to stdout (-c) */
int decompress = 0;   /* decompress (-d) */
int force = 0;        /* don't ask questions, compress links (-f) */
int no_name = -1;     /* don't save or restore the original file name */
int no_time = -1;     /* don't save or restore the original file time */
int recursive = 0;    /* recurse through directories (-r) */
int list = 0;         /* list the file contents (-l) */
int verbose = 0;      /* be verbose (-v) */
int quiet = 0;        /* be very quiet (-q) */
int do_lzw = 0;       /* generate output compatible with old compress (-Z) */
int test = 0;         /* test .gz file integrity */
int foreground;       /* set if program run in foreground */
char *progname;       /* program name */
int maxbits = BITS;   /* max bits per code for LZW */
int method = DEFLATED;/* compression method */
int level = 6;        /* compression level */
int exit_code = OK;   /* program exit code */
int save_orig_name;   /* set if original name must be saved */
int last_member;      /* set for .zip and .Z files */
int part_nb;          /* number of parts in .gz file */
long time_stamp;      /* original time stamp (modification time) */
long ifile_size;      /* input file size, -1 for devices (debug only) */
char *env;            /* contents of GZIP env variable */
char **args = NULL;   /* argv pointer if GZIP env variable defined */
char z_suffix[MAX_SUFFIX+1]; /* default suffix (can be set with --suffix) */
int  z_len;           /* strlen(z_suffix) */

long bytes_in;             /* number of input bytes */
long bytes_out;            /* number of output bytes */
long total_in = 0;         /* input bytes for all files */
long total_out = 0;        /* output bytes for all files */
char ifname[MAX_PATH_LEN]; /* input file name */
char ofname[MAX_PATH_LEN]; /* output file name */
int  remove_ofname = 0;	   /* remove output file on error */
struct stat istat;         /* status for input file */
int  ifd;                  /* input file descriptor */
int  ofd;                  /* output file descriptor */
unsigned insize;           /* valid bytes in inbuf */
unsigned inptr;            /* index of next byte to be processed in inbuf */
unsigned outcnt;           /* bytes in output buffer */

struct option longopts[] =
{
 /* { name  has_arg  *flag  val } */
    {"ascii",      0, 0, 'a'}, /* ascii text mode */
    {"to-stdout",  0, 0, 'c'}, /* write output on standard output */
    {"stdout",     0, 0, 'c'}, /* write output on standard output */
    {"decompress", 0, 0, 'd'}, /* decompress */
    {"uncompress", 0, 0, 'd'}, /* decompress */
 /* {"encrypt",    0, 0, 'e'},    encrypt */
    {"force",      0, 0, 'f'}, /* force overwrite of output file */
    {"help",       0, 0, 'h'}, /* give help */
 /* {"pkzip",      0, 0, 'k'},    force output in pkzip format */
    {"list",       0, 0, 'l'}, /* list .gz file contents */
    {"license",    0, 0, 'L'}, /* display software license */
    {"no-name",    0, 0, 'n'}, /* don't save or restore original name & time */
    {"name",       0, 0, 'N'}, /* save or restore original name & time */
    {"quiet",      0, 0, 'q'}, /* quiet mode */
    {"silent",     0, 0, 'q'}, /* quiet mode */
    {"recursive",  0, 0, 'r'}, /* recurse through directories */
    {"suffix",     1, 0, 'S'}, /* use given suffix instead of .gz */
    {"test",       0, 0, 't'}, /* test compressed file integrity */
    {"no-time",    0, 0, 'T'}, /* don't save or restore the time stamp */
    {"verbose",    0, 0, 'v'}, /* verbose mode */
    {"version",    0, 0, 'V'}, /* display version number */
    {"fast",       0, 0, '1'}, /* compress faster */
    {"best",       0, 0, '9'}, /* compress better */
    {"lzw",        0, 0, 'Z'}, /* make output compatible with old compress */
    {"bits",       1, 0, 'b'}, /* max number of bits per code (implies -Z) */
    { 0, 0, 0, 0 }
};

/* local functions */

local void treat_stdin  OF((void));
local void treat_file   OF((char *iname));
local int create_outfile OF((void));
local int  do_stat      OF((char *name, struct stat *sbuf));
local char *get_suffix  OF((char *name));
local int  get_istat    OF((char *iname, struct stat *sbuf));
local int  make_ofname  OF((void));
local int  same_file    OF((struct stat *stat1, struct stat *stat2));
local int name_too_long OF((char *name, struct stat *statb));
local void shorten_name  OF((char *name));
local int  get_method   OF((int in));
local void do_list      OF((int ifd, int method));
local int  check_ofname OF((void));
local void copy_stat    OF((struct stat *ifstat));
local void do_exit      OF((int exitcode));
      int main          OF((int argc, char **argv));
int (*work) OF((int infile, int outfile)) = zip; /* function to call */

#define strequ(s1, s2) (strcmp((s1),(s2)) == 0)

/* ========================================================================
 * Compress or decompress stdin
 */
local void treat_stdin()
{
    if (!force && !list &&
	isatty(fileno((FILE *)(decompress ? stdin : stdout)))) {
	do_exit(ERROR);
    }

    strcpy(ifname, "stdin");
    strcpy(ofname, "stdout");

    /* Get the time stamp on the input file. */
    time_stamp = 0; /* time unknown by default */

    if (list || !no_time) {
	if (fstat(fileno(stdin), &istat) != 0) {
	    error("fstat(stdin)");
	}
	    time_stamp = istat.st_mtime;
    }

    clear_bufs(); /* clear input and output buffers */
    to_stdout = 1;
    part_nb = 0;

    if (decompress) {
	method = get_method(ifd);
	if (method < 0) {
	    do_exit(exit_code); /* error message already emitted */
	}
    }
    if (list) {
        do_list(ifd, method);
        return;
    }

    for (;;) {
	if ((*work)(fileno(stdin), fileno(stdout)) != OK) return;

	if (!decompress || last_member || inptr == insize) break;
	/* end of file */

	method = get_method(ifd);
	if (method < 0) return; /* error message already emitted */
	bytes_out = 0;            /* required for length check */
    }
}

local void treat_file(iname)
    char *iname;
{
    if (strequ(iname, "-")) {
	int cflag = to_stdout;
	treat_stdin();
	to_stdout = cflag;
	return;
    }

    if (get_istat(iname, &istat) != OK) return;

    if (S_ISDIR(istat.st_mode)) {
	return;
    }
    if (!S_ISREG(istat.st_mode)) {
	return;
    }
    if (istat.st_nlink > 1 && !to_stdout && !force) {
	return;
    }

    time_stamp = no_time && !list ? 0 : istat.st_mtime;

    if (to_stdout && !list && !test) {
	strcpy(ofname, "stdout");

    } else if (make_ofname() != OK) {
	return;
    }

    ifd = OPEN(ifname, ascii && !decompress ? O_RDONLY : O_RDONLY | O_BINARY,
	       RW_USER);
    if (ifd == -1) {
	return;
    }
    clear_bufs(); /* clear input and output buffers */
    part_nb = 0;

    if (decompress) {
	method = get_method(ifd); /* updates ofname if original given */
	if (method < 0) {
	    return;               /* error message already emitted */
	}
    }
    if (list) {
        do_list(ifd, method);
        return;
    }

    if (to_stdout) {
	ofd = fileno(stdout);
    } else {
	if (create_outfile() != OK) return;
    }
    if (!save_orig_name) save_orig_name = !no_name;
    for (;;) {
	if ((*work)(ifd, ofd) != OK) {
	    method = -1; /* force cleanup */
	    break;
	}
	if (!decompress || last_member || inptr == insize) break;

	method = get_method(ifd);
	if (method < 0) break;    /* error message already emitted */
	bytes_out = 0;            /* required for length check */
    }

    if (!to_stdout && close(ofd)) {
	write_error();
    }
    if (method == -1) {
	return;
    }
    if (!to_stdout) {
	copy_stat(&istat);
    }
}

local int create_outfile()
{
    struct stat	ostat; /* stat for ofname */
    int flags = O_WRONLY | O_CREAT | O_EXCL | O_BINARY;

    if (ascii && decompress) {
	flags &= ~O_BINARY; /* force ascii text mode */
    }
    for (;;) {
	if (check_ofname() != OK) {
	    return ERROR;
	}
	/* Create the output file */
	remove_ofname = 1;
	ofd = OPEN(ofname, flags, RW_USER);
	if (ofd == -1) {
	    return ERROR;
	}

#ifdef NO_FSTAT
	if (stat(ofname, &ostat) != 0) {
#else
	if (fstat(ofd, &ostat) != 0) {
#endif
	    return ERROR;
	}
	if (!name_too_long(ofname, &ostat)) return OK;

	if (decompress) {
	    return OK;
	}
	shorten_name(ofname);
    }
}

local int do_stat(name, sbuf)
    char *name;
    struct stat *sbuf;
{
    errno = 0;
    if (!to_stdout && !force) {
	return lstat(name, sbuf);
    }
    return stat(name, sbuf);
}

local char *get_suffix(name)
    char *name;
{
    int nlen, slen;
    char suffix[MAX_SUFFIX+3]; /* last chars of name, forced to lower case */
    static char *known_suffixes[] =
       {z_suffix, ".gz", ".z", ".taz", ".tgz", "-gz", "-z", "_z",
          NULL};
    char **suf = known_suffixes;

    if (strequ(z_suffix, "z")) suf++; /* check long suffixes first */

    nlen = strlen(name);
    if (nlen <= MAX_SUFFIX+2) {
        strcpy(suffix, name);
    } else {
        strcpy(suffix, name+nlen-MAX_SUFFIX-2);
    }
    strlwr(suffix);
    slen = strlen(suffix);
    do {
       int s = strlen(*suf);
       if (slen > s && suffix[slen-s-1] != PATH_SEP
           && strequ(suffix + slen - s, *suf)) {
           return name+nlen-s;
       }
    } while (*++suf != NULL);

    return NULL;
}

local int get_istat(iname, sbuf)
    char *iname;
    struct stat *sbuf;
{
    int ilen;  /* strlen(ifname) */
    static char *suffixes[] = {z_suffix, ".gz", ".z", "-z", ".Z", NULL};
    char **suf = suffixes;
    char *s;

    strcpy(ifname, iname);

    /* If input file exists, return OK. */
    if (do_stat(ifname, sbuf) == 0) return OK;

    if (!decompress || errno != ENOENT) {
	return ERROR;
    }
    s = get_suffix(ifname);
    if (s != NULL) {
	return ERROR;
    }
    ilen = strlen(ifname);
    if (strequ(z_suffix, ".gz")) suf++;

    do {
        s = *suf;
        strcat(ifname, s);
        if (do_stat(ifname, sbuf) == 0) return OK;
	ifname[ilen] = '\0';
    } while (*++suf != NULL);
    strcat(ifname, z_suffix);
    return ERROR;
}

local int make_ofname()
{
    char *suff;            /* ofname z suffix */

    strcpy(ofname, ifname);
    suff = get_suffix(ofname);

    if (decompress) {
	if (suff == NULL) {
            if (!recursive && (list || test)) return OK;
	    return WARNING;
	}
	strlwr(suff);
	if (strequ(suff, ".tgz") || strequ(suff, ".taz")) {
	    strcpy(suff, ".tar");
	} else {
	    *suff = '\0'; /* strip the z suffix */
	}
        /* ofname might be changed later if infile contains an original name */

    } else if (suff != NULL) {
	return WARNING;
    } else {
        save_orig_name = 0;

	strcat(ofname, z_suffix);

    } /* decompress ? */
    return OK;
}

local int get_method(in)
    int in;        /* input file descriptor */
{
    uch flags;     /* compression flags */
    char magic[2]; /* magic header */
    ulg stamp;     /* time stamp */

    if (force && to_stdout) {
	magic[0] = (char)try_byte();
	magic[1] = (char)try_byte();
    } else {
	magic[0] = (char)get_byte();
	magic[1] = (char)get_byte();
    }
    method = -1;                 /* unknown yet */
    part_nb++;                   /* number of parts in gzip file */
    last_member = RECORD_IO;

    if (memcmp(magic, GZIP_MAGIC, 2) == 0
        || memcmp(magic, OLD_GZIP_MAGIC, 2) == 0) {

	method = (int)get_byte();
	if (method != DEFLATED) {
	    return -1;
	}
	work = unzip;
	flags  = (uch)get_byte();

	if ((flags & ENCRYPTED) != 0) {
	    return -1;
	}
	if ((flags & CONTINUATION) != 0) {
	    if (force <= 1) return -1;
	}
	if ((flags & RESERVED) != 0) {
	    if (force <= 1) return -1;
	}
	stamp  = (ulg)get_byte();
	stamp |= ((ulg)get_byte()) << 8;
	stamp |= ((ulg)get_byte()) << 16;
	stamp |= ((ulg)get_byte()) << 24;
	if (stamp != 0 && !no_time) time_stamp = stamp;

	(void)get_byte();  /* Ignore extra flags for the moment */
	(void)get_byte();  /* Ignore OS type for the moment */

	if ((flags & CONTINUATION) != 0) {
	    unsigned part = (unsigned)get_byte();
	    part |= ((unsigned)get_byte())<<8;
	}
	if ((flags & EXTRA_FIELD) != 0) {
	    unsigned len = (unsigned)get_byte();
	    len |= ((unsigned)get_byte())<<8;
	    while (len--) (void)get_byte();
	}

	/* Get original file name if it was truncated */
	if ((flags & ORIG_NAME) != 0) {
	    if (no_name || (to_stdout && !list) || part_nb > 1) {
		/* Discard the old name */
		char c; /* dummy used for NeXTstep 3.0 cc optimizer bug */
		do {c=get_byte();} while (c != 0);
	    } else {
		/* Copy the base name. Keep a directory prefix intact. */
                char *p = basename(ofname);
                char *base = p;
		for (;;) {
		    *p = (char)get_char();
		    if (*p++ == '\0') break;
		    if (p >= ofname+sizeof(ofname)) {
			error("corrupted input -- file name too large");
		    }
		}
                /* If necessary, adapt the name to local OS conventions: */
                if (!list) {
		   if (base) list=0; /* avoid warning about unused variable */
                }
	    } /* no_name || to_stdout */
	} /* ORIG_NAME */

	/* Discard file comment if any */
	if ((flags & COMMENT) != 0) {
	    while (get_char() != 0) /* null */ ;
	}

    } else if (memcmp(magic, PKZIP_MAGIC, 2) == 0 && inptr == 2
	    && memcmp((char*)inbuf, PKZIP_MAGIC, 4) == 0) {
	/* To simplify the code, we support a zip file when alone only.
         * We are thus guaranteed that the entire local header fits in inbuf.
         */
        inptr = 0;
	work = unzip;
	if (check_zipfile(in) != OK) return -1;
	/* check_zipfile may get ofname from the local header */
	last_member = 1;

    } else if (memcmp(magic, PACK_MAGIC, 2) == 0) {
	work = unpack;
	method = PACKED;

    } else if (memcmp(magic, LZW_MAGIC, 2) == 0) {
	work = unlzw;
	method = COMPRESSED;
	last_member = 1;

    } else if (memcmp(magic, LZH_MAGIC, 2) == 0) {
	work = unlzh;
	method = LZHED;
	last_member = 1;

    } else if (force && to_stdout && !list) { /* pass input unchanged */
	method = STORED;
	work = copy;
        inptr = 0;
	last_member = 1;
    }
    if (method >= 0) return method;

    if (part_nb == 1) {
	return -1;
    } else {
	return -2;
    }
}

/* ========================================================================
 * Display the characteristics of the compressed file.
 * If the given method is < 0, display the accumulated totals.
 * IN assertions: time_stamp, header_bytes and ifile_size are initialized.
 */
local void do_list(ifd, method)
    int ifd;     /* input file descriptor */
    int method;  /* compression method */
{
    ulg crc;  /* original crc */
    static int first_time = 1;

    if (first_time && method >= 0) {
	first_time = 0;
    } else if (method < 0) {
	return;
    }

    if (method == DEFLATED && !last_member) {
        bytes_in = (long)lseek(ifd, (off_t)(-8), SEEK_END);
        if (bytes_in != -1L) {
            uch buf[8];
            if (read(ifd, (char*)buf, sizeof(buf)) != sizeof(buf)) {
                read_error();
            }
	}
    }
}

/* ========================================================================
 * Return true if the two stat structures correspond to the same file.
 */
local int same_file(stat1, stat2)
    struct stat *stat1;
    struct stat *stat2;
{
    return stat1->st_ino   == stat2->st_ino
	&& stat1->st_dev   == stat2->st_dev
	    ;
}

local int name_too_long(name, statb)
    char *name;           /* file name to check */
    struct stat *statb;   /* stat buf for this file name */
{
    int s = strlen(name);
    char c = name[s-1];
    struct stat	tstat; /* stat for truncated name */
    int res;

    tstat = *statb;      /* Just in case OS does not fill all fields */
    name[s-1] = '\0';
    res = stat(name, &tstat) == 0 && same_file(statb, &tstat);
    name[s-1] = c;
    return res;
}

local void shorten_name(name)
    char *name;
{
    int len;                 /* length of name without z_suffix */
    char *trunc = NULL;      /* character to be truncated */
    int plen;                /* current part length */
    int min_part = MIN_PART; /* current minimum part length */
    char *p;

    len = strlen(name);
    if (decompress) {
	if (len <= 1) error("name too short");
	name[len-1] = '\0';
	return;
    }
    p = get_suffix(name);
    if (p == NULL) error("can't recover suffix\n");
    *p = '\0';
    save_orig_name = 1;

    /* compress 1234567890.tar to 1234567890.tgz */
    if (len > 4 && strequ(p-4, ".tar")) {
	strcpy(p-4, ".tgz");
	return;
    }
    /* Try keeping short extensions intact:
     * 1234.678.012.gz -> 123.678.012.gz
     */
    do {
	p = strrchr(name, PATH_SEP);
	p = p ? p+1 : name;
	while (*p) {
	    plen = strcspn(p, PART_SEP);
	    p += plen;
	    if (plen > min_part) trunc = p-1;
	    if (*p) p++;
	}
    } while (trunc == NULL && --min_part != 0);

    if (trunc != NULL) {
	do {
	    trunc[0] = trunc[1];
	} while (*trunc++);
	trunc--;
    } else {
	trunc = strrchr(name, PART_SEP[0]);
	if (trunc == NULL) error("internal error in shorten_name");
	if (trunc[1] == '\0') trunc--; /* force truncation */
    }
    strcpy(trunc, z_suffix);
}

local int check_ofname()
{
    struct stat	ostat; /* stat for ofname */

#ifdef ENAMETOOLONG
    /* Check for strictly conforming Posix systems (which return ENAMETOOLONG
     * instead of silently truncating filenames).
     */
    errno = 0;
    while (stat(ofname, &ostat) != 0) {
        if (errno != ENAMETOOLONG) return 0; /* ofname does not exist */
	shorten_name(ofname);
    }
#else
#endif
    /* Check for name truncation on existing file. Do this even on systems
     * defining ENAMETOOLONG, because on most systems the strict Posix
     * behavior is disabled by default (silent name truncation allowed).
     */
    if (!decompress && name_too_long(ofname, &ostat)) {
	shorten_name(ofname);
	if (stat(ofname, &ostat) != 0) return 0;
    }

    /* Check that the input and output files are different (could be
     * the same by name truncation or links).
     */
    if (same_file(&istat, &ostat)) {
	return ERROR;
    }
    /* Ask permission to overwrite the existing file */
    if (!force) {
	char response[80];
	strcpy(response,"n");
	if (foreground && isatty(fileno(stdin))) {
	    (void)fgets(response, sizeof(response)-1, stdin);
	}
	if (tolow(*response) != 'y') {
	    return ERROR;
	}
    }
    if (unlink(ofname)) {
	return ERROR;
    }
    return OK;
}

/* ========================================================================
 * Copy modes, times, ownership from input file to output file.
 * IN assertion: to_stdout is false.
 */
local void copy_stat(ifstat)
    struct stat *ifstat;
{
#ifndef NO_UTIME
    if (decompress && time_stamp != 0 && ifstat->st_mtime != time_stamp) {
	ifstat->st_mtime = time_stamp;
    }
#endif
    remove_ofname = 0;
}

/* ========================================================================
 * Free all dynamically allocated variables and exit with the given code.
 */
local void do_exit(exitcode)
    int exitcode;
{
    static int in_exit = 0;

    if (in_exit) exit(exitcode);
    in_exit = 1;
    exit(exitcode);
}

/* ========================================================================
 * Signal and error handler.
 */
RETSIGTYPE abort_gzip()
{
   if (remove_ofname) {
       close(ofd);
       unlink (ofname);
   }
   do_exit(ERROR);
}

int main (argc, argv)
    int argc;
    char **argv;
{
    int file_count;     /* number of files to precess */
    int proglen;        /* length of progname */
    int optc;           /* current option */

    progname = basename(argv[0]);
    proglen = strlen(progname);

    /* Suppress .exe for MSDOS, OS/2 and VMS: */
    if (proglen > 4 && strequ(progname+proglen-4, ".exe")) {
        progname[proglen-4] = '\0';
    }

    /* Add options in GZIP environment variable if there is one */
    env = add_envopt(&argc, &argv, OPTIONS_VAR);

    foreground = signal(SIGINT, SIG_IGN) != SIG_IGN;
    if (foreground) {
	(void) signal (SIGINT, (sig_type)abort_gzip);
    }
#ifdef SIGTERM
    if (signal(SIGTERM, SIG_IGN) != SIG_IGN) {
	(void) signal(SIGTERM, (sig_type)abort_gzip);
    }
#endif
#ifdef SIGHUP
    if (signal(SIGHUP, SIG_IGN) != SIG_IGN) {
	(void) signal(SIGHUP,  (sig_type)abort_gzip);
    }
#endif

#ifndef GNU_STANDARD
    if (  strncmp(progname, "un",  2) == 0     /* ungzip, uncompress */
       || strncmp(progname, "gun", 3) == 0) {  /* gunzip */
	decompress = 1;
    } else if (strequ(progname+1, "cat")       /* zcat, pcat, gcat */
	    || strequ(progname, "gzcat")) {    /* gzcat */
	decompress = to_stdout = 1;
    }
#endif

    strncpy(z_suffix, Z_SUFFIX, sizeof(z_suffix)-1);
    z_len = strlen(z_suffix);

    while ((optc = getopt_long (argc, argv, "ab:cdfhH?lLmMnNqrS:tvVZ123456789",
				longopts, (int *)0)) != EOF) {
	switch (optc) {
        case 'a':
            ascii = 1; break;
	case 'b':
	    break;
	case 'c':
	    to_stdout = 1; break;
	case 'd':
	    decompress = 1; break;
	case 'f':
	    force++; break;
	case 'h': case 'H': case '?':
	    do_exit(OK); break;
	case 'l':
	    list = decompress = to_stdout = 1; break;
	case 'L':
	    do_exit(OK); break;
	case 'm': /* undocumented, may change later */
	    no_time = 1; break;
	case 'M': /* undocumented, may change later */
	    no_time = 0; break;
	case 'n':
	    no_name = no_time = 1; break;
	case 'N':
	    no_name = no_time = 0; break;
	case 'q':
	    quiet = 1; verbose = 0; break;
	case 'r':
#ifdef NO_DIR
	    do_exit(ERROR); break;
#else
#endif
	case 'S':
            z_len = strlen(optarg);
            strcpy(z_suffix, optarg);
            break;
	case 't':
	    test = decompress = to_stdout = 1;
	    break;
	case 'v':
	    verbose++; quiet = 0; break;
	case 'V':
	    do_exit(OK); break;
	case 'Z':
#ifdef LZW
#else
	    do_exit(ERROR); break;
#endif
	case '1':  case '2':  case '3':  case '4':
	case '5':  case '6':  case '7':  case '8':  case '9':
	    level = optc - '0';
	    break;
	default:
	    do_exit(ERROR);
	}
    } /* loop on all arguments */
    if (no_time < 0) no_time = decompress;
    if (no_name < 0) no_name = decompress;

    file_count = argc - optind;

    if ((z_len == 0 && !decompress) || z_len > MAX_SUFFIX) {
        do_exit(ERROR);
    }
    if (do_lzw && !decompress) work = lzw;

    /* And get to work */
    if (file_count != 0) {
        while (optind < argc) {
	    treat_file(argv[optind++]);
	}
    } else {  /* Standard input */
    }
    return exit_code; /* just to avoid lint warning */
}

