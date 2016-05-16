#define MAXPATHLEN 1024		/* MAXPATHLEN - maximum length of a pathname we allow 	*/
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
