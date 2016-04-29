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
#define MAXPATHLEN 1024		/* MAXPATHLEN - maximum length of a pathname we allow 	*/

#ifdef UTIME_H
#	include	<utime.h>
#else
	struct utimbuf {
		time_t actime;
		time_t modtime;
	};
#endif
#	define	ARGS(a)				a
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
