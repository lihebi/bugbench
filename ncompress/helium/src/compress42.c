#include	<stdio.h>
#include	<fcntl.h>
#include	<ctype.h>
#include	<signal.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<errno.h>
#include 	<stdlib.h> //change for ccured
#include 	<string.h>
#include 	<unistd.h>
#include 	<sys/times.h>

#ifdef UTIME_H
#	include	<utime.h>
#else
	struct utimbuf {
		time_t actime;
		time_t modtime;
	};
#endif

#ifdef	__STDC__
#	define	ARGS(a)				a
#else
#	define	ARGS(a)				()
#endif

#define	LARGS(a)	()	/* Relay on include files for libary func defs. */

#ifndef SIG_TYPE
#	define	SIG_TYPE	void (*)()
#endif

#ifndef NOFUNCDEF
	extern	void	*malloc	LARGS((int));
	extern	void	free	LARGS((void *));
#ifndef _IBMR2
	//extern	int		open	LARGS((char const *,int,...));
	//changed for ccured
	extern int open (__const char *__file, int __oflag, ...);
#endif
	extern	int		close	LARGS((int));
	extern	int		chmod	LARGS((char const *,int));
	extern	int		unlink	LARGS((char const *));
	extern	int		chown	LARGS((char const *,int,int));
	extern	int		utime	LARGS((char const *,struct utimbuf const *));
	extern	char	*strcpy	LARGS((char *,char const *));
	extern	char	*strcat	LARGS((char *,char const *));
	extern	int		strcmp	LARGS((char const *,char const *));
	extern	void	*memset	LARGS((void *,char,unsigned int));
	extern	void	*memcpy	LARGS((void *,void const *,unsigned int));
	extern	int		atoi	LARGS((char const *));
	extern	void	exit	LARGS((int));
	extern	int		isatty	LARGS((int));
#endif
	
#define	MARK(a)	{ asm(" .globl M.a"); asm("M.a:"); }

#ifdef	DEF_ERRNO
	extern int	errno;
#endif

#include "patchlevel.h"

#undef	min
#define	min(a,b)	((a>b) ? b : a)

#ifndef	IBUFSIZ
#	define	IBUFSIZ	BUFSIZ	/* Defailt input buffer size							*/
#endif
#ifndef	OBUFSIZ
#	define	OBUFSIZ	BUFSIZ	/* Default output buffer size							*/
#endif

#define MAXPATHLEN 1024		/* MAXPATHLEN - maximum length of a pathname we allow 	*/
#define	SIZE_INNER_LOOP		256	/* Size of the inter (fast) compress loop			*/

							/* Defines for third byte of header 					*/
#define	MAGIC_1		(char_type)'\037'/* First byte of compressed file				*/
#define	MAGIC_2		(char_type)'\235'/* Second byte of compressed file				*/
#define BIT_MASK	0x1f			/* Mask for 'number of compresssion bits'		*/
									/* Masks 0x20 and 0x40 are free.  				*/
									/* I think 0x20 should mean that there is		*/
									/* a fourth header byte (for expansion).    	*/
#define BLOCK_MODE	0x80			/* Block compresssion if table is full and		*/
									/* compression rate is dropping flush tables	*/

			/* the next two codes should not be changed lightly, as they must not	*/
			/* lie within the contiguous general code space.						*/
#define FIRST	257					/* first free entry 							*/
#define	CLEAR	256					/* table clear output code 						*/

#define INIT_BITS 9			/* initial number of bits/code */

#ifndef SACREDMEM
	/*
 	 * SACREDMEM is the amount of physical memory saved for others; compress
 	 * will hog the rest.
 	 */
#	define SACREDMEM	0
#endif

#ifndef USERMEM
	/*
 	 * Set USERMEM to the maximum amount of physical user memory available
 	 * in bytes.  USERMEM is used to determine the maximum BITS that can be used
 	 * for compression.
	 */
#	define USERMEM 	450000	/* default user memory */
#endif

#ifndef	BYTEORDER
#	define	BYTEORDER	0000
#endif

#ifndef	NOALLIGN
#	define	NOALLIGN	0
#endif

/*
 * machine variants which require cc -Dmachine:  pdp11, z8000, DOS
 */

#ifdef interdata	/* Perkin-Elmer													*/
#	define SIGNED_COMPARE_SLOW	/* signed compare is slower than unsigned 			*/
#endif

#ifdef pdp11	 	/* PDP11: don't forget to compile with -i 						*/
#	define	BITS 		12	/* max bits/code for 16-bit machine 					*/
#	define	NO_UCHAR		/* also if "unsigned char" functions as signed char 	*/
#endif /* pdp11 */

#ifdef z8000		/* Z8000: 														*/
#	define	BITS 	12	/* 16-bits processor max 12 bits							*/
#	undef	vax			/* weird preprocessor 										*/
#endif /* z8000 */

#ifdef	DOS			/* PC/XT/AT (8088) processor									*/
#	define	BITS   16	/* 16-bits processor max 12 bits							*/
#	if BITS == 16
#		define	MAXSEG_64K
#	endif
#	undef	BYTEORDER
#	define	BYTEORDER 	4321
#	undef	NOALLIGN
#	define	NOALLIGN	1
#	define	COMPILE_DATE	__DATE__
#endif /* DOS */

#ifndef	O_BINARY
#	define	O_BINARY	0	/* System has no binary mode							*/
#endif

#ifdef M_XENIX			/* Stupid compiler can't handle arrays with */
#	if BITS == 16 		/* more than 65535 bytes - so we fake it */
# 		define MAXSEG_64K
#	else
#	if BITS > 13			/* Code only handles BITS = 12, 13, or 16 */
#		define BITS	13
#	endif
#	endif
#endif

#ifndef BITS		/* General processor calculate BITS								*/
#	if USERMEM >= (800000+SACREDMEM)
#		define FAST
#	else
#	if USERMEM >= (433484+SACREDMEM)
#		define BITS	16
#	else
#	if USERMEM >= (229600+SACREDMEM)
#		define BITS	15
#	else
#	if USERMEM >= (127536+SACREDMEM)
#		define BITS	14
#   else
#	if USERMEM >= (73464+SACREDMEM)
#		define BITS	13
#	else
#		define BITS	12
#	endif
#	endif
#   endif
#	endif
#	endif
#endif /* BITS */

#ifdef FAST
#	define	HBITS		17			/* 50% occupancy */
#	define	HSIZE	   (1<<HBITS)
#	define	HMASK	   (HSIZE-1)
#	define	HPRIME		 9941
#	define	BITS		   16
#	undef	MAXSEG_64K
#else
#	if BITS == 16
#		define HSIZE	69001		/* 95% occupancy */
#	endif
#	if BITS == 15
#		define HSIZE	35023		/* 94% occupancy */
#	endif
#	if BITS == 14
#		define HSIZE	18013		/* 91% occupancy */
#	endif
#	if BITS == 13
#		define HSIZE	9001		/* 91% occupancy */
#	endif
#	if BITS <= 12
#		define HSIZE	5003		/* 80% occupancy */
#	endif
#endif

#define CHECK_GAP 10000

typedef long int			code_int;

#ifdef SIGNED_COMPARE_SLOW
	typedef unsigned long int	count_int;
	typedef unsigned short int	count_short;
	typedef unsigned long int	cmp_code_int;	/* Cast to make compare faster	*/
#else
	typedef long int	 		count_int;
	typedef long int			cmp_code_int;
#endif

typedef	unsigned char	char_type;

#define ARGVAL() (*++(*argv) || (--argc && *++argv))

#define MAXCODE(n)	(1L << (n))

#ifndef	REGISTERS
#	define	REGISTERS	2
#endif
#define	REG1	
#define	REG2	
#define	REG3	
#define	REG4	
#define	REG5	
#define	REG6	
#define	REG7	
#define	REG8	
#define	REG9	
#define	REG10
#define	REG11	
#define	REG12	
#define	REG13
#define	REG14
#define	REG15
#define	REG16
#if REGISTERS >= 1
#	undef	REG1
#	define	REG1	register
#endif
#if REGISTERS >= 2
#	undef	REG2
#	define	REG2	register
#endif
#if REGISTERS >= 3
#	undef	REG3
#	define	REG3	register
#endif
#if REGISTERS >= 4
#	undef	REG4
#	define	REG4	register
#endif
#if REGISTERS >= 5
#	undef	REG5
#	define	REG5	register
#endif
#if REGISTERS >= 6
#	undef	REG6
#	define	REG6	register
#endif
#if REGISTERS >= 7
#	undef	REG7
#	define	REG7	register
#endif
#if REGISTERS >= 8
#	undef	REG8
#	define	REG8	register
#endif
#if REGISTERS >= 9
#	undef	REG9
#	define	REG9	register
#endif
#if REGISTERS >= 10
#	undef	REG10
#	define	REG10	register
#endif
#if REGISTERS >= 11
#	undef	REG11
#	define	REG11	register
#endif
#if REGISTERS >= 12
#	undef	REG12
#	define	REG12	register
#endif
#if REGISTERS >= 13
#	undef	REG13
#	define	REG13	register
#endif
#if REGISTERS >= 14
#	undef	REG14
#	define	REG14	register
#endif
#if REGISTERS >= 15
#	undef	REG15
#	define	REG15	register
#endif
#if REGISTERS >= 16
#	undef	REG16
#	define	REG16	register
#endif

union	bytes
{
	long	word;
	struct
	{
#if BYTEORDER == 4321
		char_type	b1;
		char_type	b2;
		char_type	b3;
		char_type	b4;
#else
#if BYTEORDER == 1234
		char_type	b4;
		char_type	b3;
		char_type	b2;
		char_type	b1;
#else
#	undef	BYTEORDER
		int				dummy;
#endif
#endif
	} bytes;
} ;

char			*progname;			/* Program name									*/
int 			silent = 0;			/* don't tell me about errors					*/
int 			quiet = 1;			/* don't tell me about compression 				*/
int				do_decomp = 0;		/* Decompress mode								*/
int				force = 0;			/* Force overwrite of files and links			*/
int				nomagic = 0;		/* Use a 3-byte magic number header,			*/
									/* unless old file 								*/
int				block_mode = BLOCK_MODE;/* Block compress mode -C compatible with 2.0*/
int				maxbits = BITS;		/* user settable max # bits/code 				*/
int 			zcat_flg = 0;		/* Write output on stdout, suppress messages 	*/
int				recursive = 0;  	/* compress directories 						*/
int				exit_code = -1;		/* Exitcode of compress (-1 no file compressed)	*/

char_type		inbuf[IBUFSIZ+64];	/* Input buffer									*/
char_type		outbuf[OBUFSIZ+2048];/* Output buffer								*/

struct stat		infstat;			/* Input file status							*/
char			*ifname;			/* Input filename								*/
int				remove_ofname = 0;	/* Remove output file on a error				*/
char 			ofname[MAXPATHLEN];	/* Output filename								*/
int				fgnd_flag = 0;		/* Running in background (SIGINT=SIGIGN)		*/

long 			bytes_in;			/* Total number of byte from input				*/
long 			bytes_out;			/* Total number of byte to output				*/

void  	main			ARGS((int,char **));
void  	comprexx		ARGS((char **));

void
main(argc, argv)
	REG1	int 	 argc;
	REG2	char	*argv[];
	{
    	REG3	char		**filelist;
		REG4	char		**fileptr;

	filelist = fileptr = (char **)malloc(argc*sizeof(fileptr[0]));

    	for (argc--, argv++; argc > 0; argc--, argv++)
		{
			if (**argv == '-')
			{/* A flag argument */
		    	while (*++(*argv))
				{/* Process all flags in this arg */
		    	}
			}
			else
			{
		    	*fileptr++ = *argv;	/* Build input file list */
		    	*fileptr = NULL;
			}

    	}
    	if (*filelist != NULL)
	{
        	for (fileptr = filelist; *fileptr; fileptr++)
			comprexx(fileptr);
    	}
}


void
comprexx(fileptr)
	char	**fileptr;
	{
		char	tempname[MAXPATHLEN];
                printf("comprexx\n");

		strcpy(tempname,*fileptr);
		//<---memory corruption happens here in strcpy
	}

