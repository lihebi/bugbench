#define MAX	2048
char tmpbuf[MAX], target[MAX], wd[MAX];
void grok_commandLine(int argc, char *argv[]){
	int o;

	while( ( o = getopt( argc, argv, "achtvf:" ) ) != -1 ){
		switch( o ){
			case 'f':
				strcpy( target, optarg );
				break;
			default:
				exit( 0 );
		}
	}

}/* end of grok_commandLine */

int main(int argc, char *argv[]){
  grok_commandLine( argc, argv );

  return( 0 );

}/* end of main */
