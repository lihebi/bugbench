#!/bin/sh
# makewhatis: create the whatis database
# Created: Sun Jun 14 10:49:37 1992
# Revised: Sat Jan  8 14:12:37 1994 by faith@cs.unc.edu
# Revised: Sat Mar 23 17:56:18 1996 by micheal@actrix.gen.nz
# Copyright 1992, 1993, 1994 Rickard E. Faith (faith@cs.unc.edu)
# May be freely distributed and modified as long as copyright is retained.
#
# Wed Dec 23 13:27:50 1992: Rik Faith (faith@cs.unc.edu) applied changes
# based on Mitchum DSouza (mitchum.dsouza@mrc-apu.cam.ac.uk) cat patches.
# Also, cleaned up code and make it work with NET-2 doc pages.
#
# makewhatis-1.4: aeb 940802, 941007, 950417
# Fixed so that the -c option works correctly for the cat pages
# on my machine. Fix for -u by Nan Zou (nan@ksu.ksu.edu).
# Many minor changes.
# The -s option is undocumented, and may well disappear again.
#
# Sat Mar 23 1996: Michael Hamilton (michael@actrix.gen.nz).
# I changed the script to invoke gawk only once for each directory tree.
# This speeds things up considerably (from 30 minutes down to 1.5 minutes
# on my 486DX66).
# 960401 - aeb: slight adaptation to work correctly with cat pages.
# 960510 - added fixes by brennan@raven.ca.boeing.com, author of mawk.
# 971012 - replaced "test -z" - it doesnt work on SunOS 4.1.3_U1.
# 980710 - be more careful with TMPFILE
#
# Note for Slackware users: "makewhatis -v -w -c" will work.
#
# makewhatis aeb 990627 (from %version%)

program=`basename $0`

PATH=/usr/bin:/bin

DEFMANPATH=/usr/man
DEFCATPATH=/usr/man/preformat:/usr/man

# AWK=/usr/bin/gawk
AWK=%gawk%

# Find a place for our temporary files. If security is not a concern, use
#	TMPFILE=/tmp/whatis$$; TMPFILEDIR=none
# Of course makewhatis should only have the required permissions
# (for reading and writing directories like /usr/man).
# We try here to be careful (and avoid preconstructed symlinks)
# in case makewhatis is run as root, by creating a subdirectory of /tmp.
# If that fails we use $HOME.
# The code below uses  test -O  which doesnt work on all systems.
TMPFILE=$HOME/whatis$$
TMPFILEDIR=/tmp/whatis$$
if [ ! -d $TMPFILEDIR ]; then
	mkdir $TMPFILEDIR
	chmod 0700 $TMPFILEDIR
	if [ -O $TMPFILEDIR ]; then
		TMPFILE=$TMPFILEDIR/w
	fi
fi

topath=manpath

defmanpath=$DEFMANPATH
defcatpath=

sections="1 2 3 4 5 6 7 8 9 n l"

for name in $*
do
if [ -n "$setsections" ]; then
	setsections=
	sections=$name
	continue
fi
case $name in
    --version|-V)
        echo "$program from %version%"
	exit 0;;
    -c) topath=catpath
	defmanpath=
	defcatpath=$DEFCATPATH
        continue;;
    -s) setsections=1
        continue;;
    -u) findarg="-ctime 0"
        update=1
        continue;;
    -v) verbose=1
	continue;;
    -w) manpath=`man --path`
	continue;;
    -*) echo "Usage: makewhatis [-u] [-v] [-w] [manpath] [-c [catpath]]"
	echo "       This will build the whatis database for the man pages"
	echo "       found in manpath and the cat pages found in catpath."
        echo "       -u: update database with new pages"
	echo "       -v: verbose"
	echo "       -w: use manpath obtained from \`man --path\`"
        echo "       [manpath]: man directories (default: $DEFMANPATH)"
	echo "       [catpath]: cat directories (default: the first existing"
	echo "           directory in $DEFCATPATH)"
        exit;;
     *) if [ -d $name ]
        then
            eval $topath="\$$topath":$name
        else
            echo "No such directory $name"
            exit
        fi;;
esac
done

manpath=`echo ${manpath-$defmanpath} | tr : ' '`
if [ x"$catpath" = x ]; then
   for d in `echo $defcatpath | tr : ' '`
   do
      if [ -d $d ]; then catpath=$d; break; fi
   done
fi
catpath=`echo ${catpath} | tr : ' '`

# first truncate all the whatis files that will be created new,
# then only update - we might visit the same directory twice
if [ x$update = x ]; then
   for pages in man cat
   do
      eval path="\$$pages"path
      for mandir in $path
      do
         cp /dev/null $mandir/whatis
      done
   done
fi

for pages in man cat
do
   export pages
   eval path="\$$pages"path
   for mandir in $path
   do
     if [ x$verbose != x ]; then
        echo "about to enter $mandir" > /dev/tty
     fi
     if [ -s ${mandir}/whatis -a $pages = man -a x$update = x ]; then
        if [ x$verbose != x ]; then
           echo skipping $mandir - we did it already > /dev/tty
        fi
     else      
       here=`pwd`
       cd $mandir
       for i in $sections
       do
         if [ -d ${pages}$i ]
         then
            cd ${pages}$i
            section=$i
            export section verbose
	    find . -name '*' $findarg -print | $AWK '

	    function readline() {
	      if (use_zcat) {
		result = (pipe_cmd | getline);
		if (result < 0) {
		  print "Pipe error: " pipe_cmd " " ERRNO > "/dev/stderr";
		}
	      } else {
		result = (getline < filename);
		if (result < 0) {
		  print "Read file error: " filename " " ERRNO > "/dev/stderr";
		}
	      }
	      return result;
	    }
	    
	    function closeline() {
	      if (use_zcat) {
		return close(pipe_cmd);
	      } else {
		return close(filename);
	      }
	    }
	    
	    function do_one() {
	      insh = 0; thisjoin = 1; done = 0;
	      entire_line = "";

	      if (verbose) {
		print "adding " filename > "/dev/tty"
	      }
	      
	      use_zcat = match(filename,"\\.Z$") ||
			 match(filename,"\\.z$") || match(filename,"\\.gz$");
	      if(use_zcat) {
	        filename_no_gz = substr(filename, 0, RSTART - 1);
	      } else {
	        filename_no_gz = filename;
	      }
	      match(filename_no_gz, "/[^/]+$");
	      progname = substr(filename, RSTART + 1, RLENGTH - 1);
	      if (match(progname, "\\." section "[A-Za-z]+")) {
		actual_section = substr(progname, RSTART + 1, RLENGTH - 1);
	      } else {
		actual_section = section;
	      }
	      sub(/\..*/, "", progname);
	      if (use_zcat) {
		pipe_cmd = "zcat " filename;
	      }
	    
	      while (!done && readline() > 0) {
		gsub(/.\b/, "");
		if (($1 ~ /^\.[Ss][Hh]/ &&
		  ($2 ~ /[Nn][Aa][Mm][Ee]/ ||
		   $2 ~ /^JM�NO/ || $2 ~ /^NAVN/ ||
		   $2 ~ /^BEZEICHNUNG/ || $2 ~ /^NOMBRE/ ||
		   $2 ~ /^NIMI/ || $2 ~ /^NOM/ || $2 ~ /^IME/ ||
		   $2 ~ /^NOME/ || $2 ~ /^NAAM/)) ||
		  (pages == "cat" && $1 ~ /^NAME/)) {
		    if (!insh) {
		      insh = 1;
		    } else {
		      done = 1;
		    }
		} else if (insh) {
		  if ($1 ~ /^\.[Ss][HhYS]/ ||
		    (pages == "cat" &&
		    ($1 ~ /^S[yYeE]/ || $1 ~ /^DESCRIPTION/ ||
		    $1 ~ /^COMMAND/ || $1 ~ /^OVERVIEW/ ||
		    $1 ~ /^STRUCTURES/ || $1 ~ /^INTRODUCTION/))) {
		      # end insh for Synopsis, Syntax, but also for
		      # DESCRIPTION (e.g., XFree86.1x),
		      # COMMAND (e.g., xspread.1)
		      # OVERVIEW (e.g., TclCommandWriting.3)
		      # STRUCTURES (e.g., XEvent.3x)
		      # INTRODUCTION (e.g., TclX.n)
		    done = 1;
		  } else {
		    if (!after && $0 ~ progname"-") {  # Fix old cat pages
		        sub(progname"-", progname" - ");
		    }
	            if ($0 ~ /[^ ]-$/) {
		      sub(/-$/, "");          # Handle Hyphenations
		      nextjoin = 1;
		    } else if ($0 ~ /\\c$/) {
	              sub(/\\c$/, "");        # Handle Continuations
		      nextjoin = 1;
		    } else
		      nextjoin = 0;

		    sub(/^.[IB] /, "");       # Kill bold and italics
		    sub(/^.SM /, "");         # Kill small
		    sub(/^.Nm /, "");         # Kill bold
		    sub(/^.Tn /, "");         # Kill normal
	            sub(/^.Li /, "");         # Kill .Li
	            sub(/^.Dq /, "");         # Kill .Dq
	            sub(/^.Nd */, "- ");      # Convert .Nd to dash
	            sub(/^\.\\\".*/, "");     # Kill comments
		    sub(/^\. *$/, "");        # Kill blank comments (curs_bkgd.3x)
		    sub(/^'"'"'.*/, "");      # Kill comment/troff lines (gsftopk.1, Tree.n)

		    if (($1 ~ /^\.../ || $1 == "") &&
		        (entire_line ~ / - / || entire_line ~ / \\- /)) {
		      # Assume that this ends the description of one line
		      # Sometimes there are several descriptions in one page,
		      # as in outb(2).
		      handle_entire_line();
		      entire_line = "";
		      thisjoin = 1;
		    } else {
		      if (thisjoin) {
		        entire_line = entire_line $0;
		      } else {
		        entire_line = entire_line " " $0;
		      }
		      thisjoin = nextjoin;
		    }
		  }
		}
	      }
	      handle_entire_line();
	      closeline();
	    }

	    function handle_entire_line() {
	      x = entire_line;             # Keep it short

	      gsub(/	/, " ", x);        # Translate tabs to spaces
	      gsub(/  +/, " ", x);         # Collapse spaces
	      gsub(/ *, */, ", ", x);      # Fix comma spacings
	      sub(/^ /, "", x);            # Kill initial spaces
	      sub(/ $/, "", x);            # Kill trailing spaces
	      sub(/__+/, "_", x);          # Collapse underscores

	      gsub(/\\f\(../, "", x);         # Kill font changes
	      gsub(/\\f[PRIB0123]/, "", x);   # Kill font changes
	      gsub(/\\s[-+0-9]*/, "", x);     # Kill size changes
	      gsub(/\\&/, "", x);             # Kill \&
	      gsub(/\\\|/, "", x);            # Kill \|
	      gsub(/\\\((ru|ul)/, "_", x);    # Translate
	      gsub(/\\\((mi|hy|em)/, "-", x); # Translate
	      gsub(/\\\*\(../, "", x);        # Kill troff strings
	      gsub(/\\/, "", x);              # Kill all backslashes
	      gsub(/"/, "", x);               # Kill quotes (from .Nd "foo bar")
	      sub(/\.$/, "", x);              # Kill trailing period (dlopen(3))

	      if (!match(x, / - /))
	        return;

	      after_dash = substr(x, RSTART);
	      head = substr(x, 1, RSTART-1) ", ";
	      while (match(head, /, /)) {
	        prog = substr(head, 1, RSTART-1);
		head = substr(head, RSTART+2);
		if (prog != progname)
		  prog = prog " [" progname "]";
		printf "%-*s (%s) %s\n", 20, prog, actual_section, after_dash;
	      }
	    }

            {			# Main action - process each filename read in.
	      filename = $0;
	      do_one();
	    }
	    ' pages=$pages section=$section verbose=$verbose
            cd ..
         fi
       done > $TMPFILE

       cd $here

       # kludge for Slackware's /usr/man/preformat
       if [ $mandir = /usr/man/preformat ]
       then
         mandir1=/usr/man
       else
         mandir1=$mandir
       fi

       if [ -f ${mandir1}/whatis ]
       then
         cat ${mandir1}/whatis >> $TMPFILE
       fi
       sed '/^$/d' < $TMPFILE | sort | uniq > ${mandir1}/whatis

       chmod 644 ${mandir1}/whatis
       rm $TMPFILE
     fi
   done
done

# remove the dir if we created it
if [ $TMPFILE = $TMPFILEDIR/w ]; then
	rmdir $TMPFILEDIR
fi
