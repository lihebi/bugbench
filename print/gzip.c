int main (argc, argv)
    int argc;
    char **argv;
{
    int file_count;     /* number of files to precess */
    int proglen;        /* length of progname */
    int optc;           /* current option */

    EXPAND(argc, argv); /* wild card expansion if necessary */

    progname = basename(argv[0]);
    proglen = strlen(progname);

    /* Suppress .exe for MSDOS, OS/2 and VMS: */
    if (proglen > 4 && strequ(progname+proglen-4, ".exe")) {
        progname[proglen-4] = '\0';
    }

    /* Add options in GZIP environment variable if there is one */
    env = add_envopt(&argc, &argv, OPTIONS_VAR);
    if (env != NULL) args = argv;

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
	    maxbits = atoi(optarg);
	    break;
	case 'c':
	    to_stdout = 1; break;
	case 'd':
	    decompress = 1; break;
	case 'f':
	    force++; break;
	case 'h': case 'H': case '?':
	    help(); do_exit(OK); break;
	case 'l':
	    list = decompress = to_stdout = 1; break;
	case 'L':
	    license(); do_exit(OK); break;
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
	    fprintf(stderr, "%s: -r not supported on this system\n", progname);
	    usage();
	    do_exit(ERROR); break;
#else
	    recursive = 1; break;
#endif
	case 'S':
#ifdef NO_MULTIPLE_DOTS
            if (*optarg == '.') optarg++;
#endif
            z_len = strlen(optarg);
            strcpy(z_suffix, optarg);
            break;
	case 't':
	    test = decompress = to_stdout = 1;
	    break;
	case 'v':
	    verbose++; quiet = 0; break;
	case 'V':
	    version(); do_exit(OK); break;
	case 'Z':
#ifdef LZW
	    do_lzw = 1; break;
#else
	    fprintf(stderr, "%s: -Z not supported in this version\n",
		    progname);
	    usage();
	    do_exit(ERROR); break;
#endif
	case '1':  case '2':  case '3':  case '4':
	case '5':  case '6':  case '7':  case '8':  case '9':
	    level = optc - '0';
	    break;
	default:
	    /* Error message already emitted by getopt_long. */
	    usage();
	    do_exit(ERROR);
	}
    } /* loop on all arguments */

    /* By default, save name and timestamp on compression but do not
     * restore them on decompression.
     */
    if (no_time < 0) no_time = decompress;
    if (no_name < 0) no_name = decompress;

    file_count = argc - optind;

#if O_BINARY
#else
    if (ascii && !quiet) {
	fprintf(stderr, "%s: option --ascii ignored on this system\n",
		progname);
    }
#endif
    if ((z_len == 0 && !decompress) || z_len > MAX_SUFFIX) {
        fprintf(stderr, "%s: incorrect suffix '%s'\n",
                progname, optarg);
        do_exit(ERROR);
    }
    if (do_lzw && !decompress) work = lzw;

    /* Allocate all global buffers (for DYN_ALLOC option) */
    ALLOC(uch, inbuf,  INBUFSIZ +INBUF_EXTRA);
    ALLOC(uch, outbuf, OUTBUFSIZ+OUTBUF_EXTRA);
    ALLOC(ush, d_buf,  DIST_BUFSIZE);
    ALLOC(uch, window, 2L*WSIZE);
#ifndef MAXSEG_64K
    ALLOC(ush, tab_prefix, 1L<<BITS);
#else
    ALLOC(ush, tab_prefix0, 1L<<(BITS-1));
    ALLOC(ush, tab_prefix1, 1L<<(BITS-1));
#endif

    /* And get to work */
    if (file_count != 0) {
	if (to_stdout && !test && !list && (!decompress || !ascii)) {
	    SET_BINARY_MODE(fileno(stdout));
	}
        while (optind < argc) {
	    treat_file(argv[optind++]);
	}
    } else {  /* Standard input */
	treat_stdin();
    }
    if (list && !quiet && file_count > 1) {
	do_list(-1, -1); /* print totals */
    }
    do_exit(exit_code);

    return exit_code; /* just to avoid lint warning */
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
	WARN((stderr,"%s: %s is a directory -- ignored\n", progname, ifname));
	return;
    }
    if (!S_ISREG(istat.st_mode)) {
	WARN((stderr,
	      "%s: %s is not a directory or a regular file - ignored\n",
	      progname, ifname));
	return;
    }
    if (istat.st_nlink > 1 && !to_stdout && !force) {
	WARN((stderr, "%s: %s has %d other link%c -- unchanged\n",
	      progname, ifname,
	      (int)istat.st_nlink - 1, istat.st_nlink > 2 ? 's' : ' '));
	return;
    }

    ifile_size = istat.st_size;
    time_stamp = no_time && !list ? 0 : istat.st_mtime;
    if (to_stdout && !list && !test) {
	strcpy(ofname, "stdout");

    } else if (make_ofname() != OK) {
	return;
    }
    ifd = OPEN(ifname, ascii && !decompress ? O_RDONLY : O_RDONLY | O_BINARY,
	       RW_USER);
    if (ifd == -1) {
	fprintf(stderr, "%s: ", progname);
	perror(ifname);
	exit_code = ERROR;
	return;
    }
    clear_bufs(); /* clear input and output buffers */
    part_nb = 0;

    if (decompress) {
	method = get_method(ifd); /* updates ofname if original given */
	if (method < 0) {
	    close(ifd);
	    return;               /* error message already emitted */
	}
    }
    if (list) {
        do_list(ifd, method);
        close(ifd);
        return;
    }
    if (to_stdout) {
	ofd = fileno(stdout);
    } else {
	if (create_outfile() != OK) return;

	if (!decompress && save_orig_name && !verbose && !quiet) {
	    fprintf(stderr, "%s: %s compressed to %s\n",
		    progname, ifname, ofname);
	}
    }
    if (!save_orig_name) save_orig_name = !no_name;

    if (verbose) {
	fprintf(stderr, "%s:\t%s", ifname, (int)strlen(ifname) >= 15 ? 
		"" : ((int)strlen(ifname) >= 7 ? "\t" : "\t\t"));
    }
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
    close(ifd);
    if (!to_stdout && close(ofd)) {
	write_error();
    }
    if (method == -1) {
	if (!to_stdout) unlink (ofname);
	return;
    }
    if(verbose) {
	if (test) {
	    fprintf(stderr, " OK");
	} else if (decompress) {
	    display_ratio(bytes_out-(bytes_in-header_bytes), bytes_out,stderr);
	} else {
	    display_ratio(bytes_in-(bytes_out-header_bytes), bytes_in, stderr);
	}
	if (!test && !to_stdout) {
	    fprintf(stderr, " -- replaced with %s", ofname);
	}
	fprintf(stderr, "\n");
    }
    if (!to_stdout) {
	copy_stat(&istat);
    }
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
    if (do_stat(ifname, sbuf) == 0) return OK;
    if (!decompress || errno != ENOENT) {
	perror(ifname);
	exit_code = ERROR;
	return ERROR;
    }
    s = get_suffix(ifname);
    if (s != NULL) {
	perror(ifname); /* ifname already has z suffix and does not exist */
	exit_code = ERROR;
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
    perror(ifname);
    exit_code = ERROR;
    return ERROR;
}

