#define MAXPATHLEN 1024		/* MAXPATHLEN - maximum length of a pathname we allow 	*/
#define	SIZE_INNER_LOOP		256	/* Size of the inter (fast) compress loop			*/

void
main(argc, argv)
	REG1	int 	 argc;
	REG2	char	*argv[];
	{
    	REG3	char		**filelist;
		REG4	char		**fileptr;
    	if (fgnd_flag = (signal(SIGINT, SIG_IGN) != SIG_IGN))
			signal(SIGINT, (SIG_TYPE)abort_compress);
		signal(SIGTERM, (SIG_TYPE)abort_compress);
	filelist = fileptr = (char **)malloc(argc*sizeof(fileptr[0]));
    	*filelist = NULL;
    	if((progname = rindex(argv[0], '/')) != 0)
			progname++;
		else
			progname = argv[0];
    	if (strcmp(progname, "uncompress") == 0)
			do_decomp = 1;
		else
		if (strcmp(progname, "zcat") == 0)
			do_decomp = zcat_flg = 1;

    	for (argc--, argv++; argc > 0; argc--, argv++)
		{
			if (**argv == '-')
			{/* A flag argument */
		    	while (*++(*argv))
				{/* Process all flags in this arg */
					switch (**argv)
					{
			    	case 'V':
						about();
						break;

					case 's':
						silent = 1;
						quiet = 1;
						break;

			    	case 'v':
						silent = 0;
						quiet = 0;
						break;

			    	case 'd':
						do_decomp = 1;
						break;

			    	case 'f':
			    	case 'F':
						force = 1;
						break;

			    	case 'n':
						nomagic = 1;
						break;

			    	case 'C':
						block_mode = 0;
						break;

			    	case 'b':
						if (!ARGVAL())
						{
					    	fprintf(stderr, "Missing maxbits\n");
					    	Usage();
						}

						maxbits = atoi(*argv);
						goto nextarg;

		    		case 'c':
						zcat_flg = 1;
						break;

			    	case 'q':
						quiet = 1;
						break;
			    	case 'r':
			    	case 'R':
						break;
			    	default:
						fprintf(stderr, "Unknown flag: '%c'; ", **argv);
						Usage();
					}
		    	}
			}
			else
			{
		    	*fileptr++ = *argv;	/* Build input file list */
		    	*fileptr = NULL;
			}

nextarg:	continue;
    	}

    	if (maxbits < INIT_BITS)	maxbits = INIT_BITS;
    	if (maxbits > BITS) 		maxbits = BITS;
    	if (*filelist != NULL)
	{
        	for (fileptr = filelist; *fileptr; fileptr++)
			comprexx(fileptr);
    	}
	else
	{/* Standard input */
		ifname = "";
		exit_code = 0;
		remove_ofname = 0;
		if (do_decomp == 0)
		{
			compress(0, 1);

			if (zcat_flg == 0 && !quiet)
			{
				fprintf(stderr, "Compression: ");
				prratio(stderr, bytes_in-bytes_out, bytes_in);
				fprintf(stderr, "\n");
			}

			if (bytes_out >= bytes_in && !(force))
				exit_code = 2;
		}
		else
			decompress(0, 1);
	}
	fprintf(stdout,"compress finish\n");
	exit((exit_code== -1) ? 1:exit_code);
}

void
comprexx(fileptr)
	char	**fileptr;
	{
		int		fdin;
		int		fdout;
		char	tempname[MAXPATHLEN];

		strcpy(tempname,*fileptr);
		//<---memory corruption happens here in strcpy
		errno = 0;

	}

