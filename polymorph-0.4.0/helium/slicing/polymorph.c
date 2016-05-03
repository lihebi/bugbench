/*========================================================
 * polymorph -- a Win32 -> Unix filename convertor
 * Copyright 1999 - 2001 by Gerall Kahla
 * <kahlage@users.sourceforge.net>
 *========================================================
 * boilerplate here
 *========================================================
 * please see the end of this file for revision history
 *========================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "polymorph_types.h"
#include "polymorph.h"

/* global variables & defines */
char tmpbuf[MAX], target[MAX], wd[MAX];
struct dirent *victim;
struct stat status;
DIR *curr_dir;
int	hidden = 0;	/* by default, do NOT convert hidden files */
int	track = 0;	/* by default, do NOT track symlinks */
int	clean = 0;	/* by default, do NOT clean backslashed names */
int main(int argc, char *argv[]){
  grok_commandLine( argc, argv );
}/* end of main */
void grok_commandLine(int argc, char *argv[]){
	int o;

	while( ( o = getopt( argc, argv, "achtvf:" ) ) != -1 ){
		switch( o ){
			case 'a':
				break;
			case 'c':
				break;
			case 'f':
				strcpy( target, optarg );
				break;
			case 'h':
				exit( 0 );
			case 't':
				break;
			case 'v':
				exit( 0 );
			default:
				exit( 0 );
		}
	}

}/* end of grok_commandLine */
