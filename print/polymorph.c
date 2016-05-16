/* global variables & defines */
char tmpbuf[MAX], target[MAX], wd[MAX];
struct dirent *victim;
struct stat status;
DIR *curr_dir;
int	hidden = 0;	/* by default, do NOT convert hidden files */
int	track = 0;	/* by default, do NOT track symlinks */
int	clean = 0;	/* by default, do NOT clean backslashed names */

int main(int argc, char *argv[]){
  char filename[MAX];
  strcpy( target, "" );
  grok_commandLine( argc, argv );
  if( strlen(target) != 0 ){
    convert_fileName( target );
    goto return_0;
  }

  /*   move_toNewDir( target ); */

  strcpy( wd, "" );
  strcpy( filename, "" );

  getcwd( wd, sizeof( wd ) );

  curr_dir = opendir( wd );
  if( curr_dir == NULL ){
    fprintf( stderr, "polymorph could not open the current working directory\n" );
    fprintf( stderr, "maybe you don't have permissions?\n" );
    fprintf( stderr, "polymorph terminated\n" );
    exit( 1 );
  }

  while( ( victim = readdir( curr_dir ) ) != NULL ){
    /* check to see if victim is a regular file */
    if( track ){
      /* work on the actual file */
      if( stat( victim->d_name, &status ) == -1 ){
        fprintf( stderr,"polymorph encountered something funky\n" );
        fprintf( stderr,"polymorph terminated\n" );
        return( 2 );
      }
      if( S_ISREG( status.st_mode ) ){
        strcpy( filename, victim->d_name );
        convert_fileName( filename );
        /* move_toNewDir( filename ); */
      }
    }else{
      /* work on the symlink to the file */
      if( lstat( victim->d_name, &status ) == -1 ){
        fprintf( stderr,"polymorph encountered something funky\n" );
        fprintf( stderr,"polymorph terminated\n" );
        return( 2 );
      }
      if( S_ISREG( status.st_mode ) ){
        strcpy( filename, victim->d_name );
        convert_fileName( filename );
        /* move_toNewDir( filename ); */
      }
    }
  }

  closedir( curr_dir );

return_0:

  return( 0 );

}/* end of main */
void grok_commandLine(int argc, char *argv[]){
	int o;

	while( ( o = getopt( argc, argv, "achtvf:" ) ) != -1 ){
		switch( o ){
			case 'a':
				hidden = 1;
				break;
			case 'c':
				clean = 1;
				break;
			case 'f':
				strcpy( target, optarg );
				break;
			case 'h':
				fprintf( stderr,"polymorph v%s -- filename convertor\n", VERSION );
        fprintf( stderr,"written by Gerall Kahla.\n\n" );
				fprintf( stderr,"-a  all      convert hidden files\n" );
				fprintf( stderr,"-c	clean		 reduce a file's name to just after the last backslash\n" );
				fprintf( stderr,"-f  file     convert this file to a name with all lowercase letters\n" );
				fprintf( stderr,"-h  help     print this message and exit\n" );
				fprintf( stderr,"-t  track    track down the targets of symlinks and convert them\n" );
				fprintf( stderr,"-v  version  print the version number and exit\n" );
				fprintf( stderr,"\n" );
				exit( 0 );
			case 't':
				track = 1;
				break;
			case 'v':
				fprintf( stderr,"polymorph v%s\n", VERSION );
				exit( 0 );
			default:
				fprintf( stderr,"please run 'polymorph -h' for commandline options\n" );
				fprintf( stderr,"polymorph terminated\n" );
				exit( 0 );
		}
	}

}/* end of grok_commandLine */
