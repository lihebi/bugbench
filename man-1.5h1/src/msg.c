char *msg[] = {
  "",
/*  1 */  "unable to make sense of the file %s\n",
/*  2 */  "Warning: cannot open configuration file %s\n",
/*  3 */  "Error parsing config file\n",
/*  4 */  "incompatible options %s and %s\n",
/*  5 */  "Sorry - no support for alternate systems compiled in\n",
/*  6 */  "Man was compiled with automatic cat page compression,\n\
but the configuration file does not define COMPRESS.\n",
/*  7 */  "What manual page do you want from section %s?\n",
/*  8 */  "What manual page do you want?\n",
/*  9 */  "No entry for %s in section %s of the manual\n",
/* 10 */  "No manual entry for %s\n",
/* 11 */  "\nusing %s as pager\n",
/* 12 */  "Error executing formatting or display command.\n\
System command %s exited with status %d.\n",
/* 13 */  "%s, version %s\n\n",
/* 14 */  "Out of memory - can't malloc %d bytes\n",
/* 15 */  "Error parsing *roff command from file %s\n",
/* 16 */  "Error parsing MANROFFSEQ.  Using system defaults.\n",
/* 17 */  "Error parsing *roff command from command line.\n",
/* 18 */  "Unrecognized line in config file (ignored)\n%s\n",
/* 19 */  "man-config.c: internal error: string %s not found\n",
/* 20 */  "found man directory %s\n",
/* 21 */  "found manpath map %s --> %s\n",
/* 22 */  "corresponding catdir is %s\n",
/* 23 */  "Line too long in config file\n",
/* 24 */  "\nsection: %s\n",
/* 25 */  "unlinked %s\n",
/* 26 */  "globbing %s\n",
/* 27 */  "Attempt [%s] to expand man page failed\n",
/* 28 */  "Cannot open man page %s\n",
/* 29 */  "Error reading man page %s\n",
/* 30 */  "found eqn(1) directive\n",
/* 31 */  "found grap(1) directive\n",
/* 32 */  "found pic(1) directive\n",
/* 33 */  "found tbl(1) directive\n",
/* 34 */  "found vgrind(1) directive\n",
/* 35 */  "found refer(1) directive\n",
/* 36 */  "parsing directive from command line\n",
/* 37 */  "parsing directive from file %s\n",
/* 38 */  "parsing directive from environment\n",
/* 39 */  "using default preprocessor sequence\n",
/* 40 */  "Formatting page, please wait...\n",
/* 41 */  "changed mode of %s to %o\n",
/* 42 */  "Couldn't open %s for writing.\n",
/* 43 */  "will try to write %s if needed\n",
/* 44 */  "status from is_newer() = %d\n",
/* 45 */  "trying section %s\n",
/* 46 */  "\nsearching in %s\n",
/* 47 */  "but %s is already in the manpath\n",
/* 48 */  "Warning: cannot stat file %s!\n",
/* 49 */  "Warning: %s isn't a directory!\n",
/* 50 */  "adding %s to manpath\n",
/* 51 */  "\npath directory %s ",
/* 52 */  "is in the config file\n",
/* 53 */  "is not in the config file\n",
/* 54 */  "but there is a man directory nearby\n",
/* 55 */  "and we found no man directory nearby\n",
/* 56 */  "\nadding mandatory man directories\n\n",
/* 57 */  "cat_name in convert_to_cat () is: %s\n",
/* 58 */  "\nnot executing command:\n  %s\n",
/* 59 */  "usage: %s [-adfhktwW] [section] [-M path] [-P pager] [-S list]\n\t",
/* 60 */  "[-m system] ",
/* 61 */  "[-p string] name ...\n\n",
/* 62 */  "  a : find all matching entries\n\
  c : do not use cat file\n\
  d : print gobs of debugging information\n\
  D : as for -d, but also display the pages\n\
  f : same as whatis(1)\n\
  h : print this help message\n\
  k : same as apropos(1)\n\
  K : search for a string in all pages\n",
/* 63 */  "  t : use troff to format pages for printing\n",
/* 64 */  "\
  w : print location of man page(s) that would be displayed\n\
      (if no name given: print directories that would be searched)\n\
  W : as for -w, but display filenames only\n\n\
  C file   : use `file' as configuration file\n\
  M path   : set search path for manual pages to `path'\n\
  P pager  : use program `pager' to display pages\n\
  S list   : colon separated section list\n",
/* 65 */  "  m system : search for alternate system's man pages\n",
/* 66 */  "  p string : string tells which preprocessors to run\n\
               e - [n]eqn(1)   p - pic(1)    t - tbl(1)\n\
               g - grap(1)     r - refer(1)  v - vgrind(1)\n",
/* 67 */  "and the real user cannot open the cat file either\n",
/* 68 */  "but the real user can open the cat file\n",
/* 69 */  "failed to fork off the command _%s_\n",
/* 70 */  "error while waiting for child _%s_\n",
/* 71 */  "very strange ..., got wrong pid while waiting for my child\n",
/* 72 */  "fatal error: the command _%s_ terminated abnormally\n",
/* 73 */  "Man page %s is identical to %s\n",
/* 74 */  "Found the man page(s):\n",
/* 75 */  "error: no TROFF command specified in %s\n",
/* 76 */  "no cat page stored because of nonstandard line length\n",
};
