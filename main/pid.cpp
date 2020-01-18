
#include "pid.h"

#define EXEC_MAX_STACK 256

const char * home = getenv("HOME");

pid_t execStack[ EXEC_MAX_STACK ] = { 0 };

/*void initPidStack( void ){
  memset( execStack, 0, EXEC_MAX_STACK * sizeof( pid_t ) );
}*/

void pushPid( pid_t pid ){
  u32 n = 0;
  while( execStack[n] && n < EXEC_MAX_STACK ) n++;
  if( n == EXEC_MAX_STACK ) return;
  execStack[ n ] = pid;
}

void refreshPidStack( void ){
  char * path = (char*)malloc( 256 );
  if( !path ) return;

  strcpy( path, "/proc/" );
  char * p = path + 6;

  struct stat st;
  pid_t *pid = execStack, *end = pid + EXEC_MAX_STACK ;

  while( pid != end ){
    if( !*pid ){
      pid++; continue;
    }

    sprintf( p, "%i", *pid );
    //itoa( *pid, p, 10 );
    if( stat( path, &st ) != 0 ){
      // not exist, clear
      *pid++ = 0;
      continue;
    }

    // exist, check isdir
    if( !S_ISDIR(st.st_mode) ){
      // not a dir, clear
      *pid++ = 0;
      continue;
    }

    // process still present

    pid++;
  }

  free( path );
}

struct list * pidStack2list( void ) {
//  struct list * l = newList( 0,0 );
  char * path = (char*)malloc( 512 );
  if( !path ) return 0;

  char *labels = (char*)malloc( 64*1024 ), *label = labels;
  char ** content = (char**)malloc( EXEC_MAX_STACK * sizeof( char* ) );
  u32 nb = 0;

  char *spid = path + 200;
  char * cmd = path + 255;

  strcpy( path, "/proc/" );
  char * p = path + 6;

  struct stat st;
  pid_t *pid = execStack, *end = pid + EXEC_MAX_STACK ;

  while( pid != end ){
    if( !*pid ){
      pid++; continue;
    }

//    printf("check pid %i\n", *pid );

    sprintf( spid, "%i", *pid );
    sprintf( p, "%s/cmdline", spid );

//    printf("check file %s\n", path);

    if( stat( path, &st ) != 0 ){
      // not exist, clear
      *pid++ = 0;
      continue;
    }

    // exist, load it

    FILE * f = fopen( path , "r");
    if(!f){
      *pid++ = 0; continue;
    };

    size_t l = fread( cmd, 1, 256, f );
    fclose( f );

//    printf("read %i bytes\n",l);

    if( !l ){
      *pid++ = 0;
      continue;
    }

    for( u32 n=0; n<l; n++ ) if( !cmd[n] ) cmd[n] = ' ';
    cmd[ l ] = 0;

    content[ nb ] = label;
    char * c = spid;
    while( *c ) *label++ = *c++;
    *label++ = ' ';
    *label++ = '/';
    *label++ = ' ';
    c = cmd;
    while( *label++ = *c++ );

//    printf("content %s\n", content[nb]);

    nb++;
    pid++;
  }

  free( path );

  content[ nb ] = 0;
  struct list * l = string2List( (const char**)content );

  l->labels = labels;

  free( content );
  return l;
}


void exec( const char * cmd, int push ){
  const char * path = cmd;
  char * pathEnd = 0;

  char *fileName = (char*)malloc(2048), *f = fileName;

  //printf("%s\n",cmd);
  char *fPtr = (char*)cmd;
  while( *fPtr ){
    if( *fPtr == '/' ) pathEnd = fPtr;
    *f++ = *fPtr++;
  };

  *f = 0;
  if( pathEnd ) *pathEnd = 0; else path = 0;

  //printf("%s\n",path);

  const char * cmds[] = { fileName, NULL };
  _exec( cmds, push, path );
  free(fileName);
}

/*void exec( const char * cmd ){
	//static const char * home = getenv("HOME");
	pid_t pid = fork();
	if( pid < 0 ){ // error forking
			printf("fork failed\n");
	} else {
		if( pid ){ // parent
			// do nothing ..
			printf("exec %s\n",cmd);
		} else { // child
			setsid() ; // become session leader
			chdir( home ) ; // change the working dir
			umask( 0 ) ;   // clear out the file mode creation mask
			if( !cmd ) exit(EXIT_FAILURE);
			char *argv[2];
			argv[0] = (char*)cmd;
			argv[1] = NULL;
			int err = execvp( argv[0] , argv );
			exit( err ? EXIT_FAILURE : EXIT_SUCCESS );
		}
	}
}*/


#define MAX_EXEC_LINES 256
#define MAX_EXEC_BUFFER 4096
/*
int _exec( const char ** argv, int push, const char *dir ){
	//static const char * home = getenv("HOME");
  if( !argv || !*argv ) return -1;
//  int fd[2];
//  pipe( fd );

	pid_t pid = fork();
	if( pid < 0 ) return -1;
  if( pid ){  // main process
    if( push ) pushPid( pid );

    char * pidlog = (char*)malloc(1024);
    char *b = (char *)malloc( MAX_EXEC_BUFFER );

    sprintf(pidlog,"/proc/%i/fd/1",pid);
    printf("pidlog file %s\n",pidlog);

    FILE *f = fopen( pidlog, "r" );
    while( 1 ){
      char *s = fgets( b, 1023, f );
      if( !s ) break;
      printf("<%s>\n",s);
    };
    fclose(f);

    free(b);
    free(pidlog);

    return 1;
  }

  // child process
//    dup2( fd[1], 1 ); // redirect out
//    dup2( fd[1], 2 ); // redirect out
//    close( fd[0] ); // close in
//    close( fd[1] ); // close out

	setsid() ;
	chdir( dir ? dir : home ) ;
	umask( 0 ) ;
	exit( execvp( *argv , (char * const *)argv ) ? EXIT_FAILURE : EXIT_SUCCESS );
}
*/

char * p_exec( const char ** argv ){
  if( !argv || !*argv ) return NULL;

  int fd[2];
  pipe( fd );

	pid_t pid = fork();
	if( pid < 0 ) return NULL;
  if( pid ){  // main process
    //dup2( fd[0], 0 ); // read in
    close( fd[1] ); // close out

    char *bf = (char *)malloc( MAX_EXEC_BUFFER ), *b = bf;
    *b = 0;
    //printf("exec %s pid %i\n", *argv, pid);
    FILE *f = fdopen( fd[0], "r" );
    while( 1 ){
      char *s = fgets( b, 256, f );
      if( !s ) break;
      //printf("<%s>\n",s);
      u32 nb = strlen( b );
      b += nb;// - 1; *b = 0;
    };
    if( b != bf ){ b--; *b++ = 0; } // strip last \n
    fclose(f);
    close( fd[0] );

    /*char * o = (char *)realloc( bf, b - bf );
    if( o ) bf = o;*/

    return bf;
  }

  // child process
  dup2( fd[1], 1 ); // redirect out
  dup2( fd[1], 2 ); // redirect out
  close( fd[0] ); // close in
  close( fd[1] ); // close out

	setsid() ;   /* become session leader */
	chdir( home ) ; /* change the working dir */
	umask( 0 ) ;   /* clear out the file mode creation mask */

  int out = execvp( *argv , (char * const *)argv );

  exit( out ? EXIT_FAILURE : EXIT_SUCCESS );
}

char ** pipe_exec( const char ** argv ){
  if( !argv || !*argv ) return NULL;

  int fd[2];
  pipe( fd );

	pid_t pid = fork();
	if( pid < 0 ) return NULL;
  if( pid ){  // main process
    //dup2( fd[0], 0 ); // read in
    close( fd[1] ); // close out

    char ** out = (char **)malloc( MAX_EXEC_LINES * sizeof(char*) + MAX_EXEC_BUFFER );
    char *bf = ( (char*)out ) + ( MAX_EXEC_LINES * sizeof(char*) ), *b = bf;
    //printf("exec %s pid %i\n", *argv, pid);
    u32 n = 0;

    FILE *f = fdopen( fd[0], "r" );
    while( 1 ){
      char *s = fgets( b, 256, f );
      if( !s ) break;
      //printf("%s",s);
      out[ n++ ] = b;
      u32 nb = strlen( b );
      b += nb+1;
    };
    fclose(f);
    out[ n ] = NULL;

    //if( push ) pushPid( pid );
    return out;
  }

  // child process
  dup2( fd[1], 1 ); // redirect out
  dup2( fd[1], 2 ); // redirect out
  close( fd[0] ); // close in
  close( fd[1] ); // close out

	setsid() ;   /* become session leader */
	chdir( home ) ; /* change the working dir */
	umask( 0 ) ;   /* clear out the file mode creation mask */
	exit( execvp( *argv , (char * const *)argv ) ? EXIT_FAILURE : EXIT_SUCCESS );
}

int callback_exec( const char ** argv, void (*fn)(char*) ){
  if( !argv || !*argv ) return 0;

  int fd[2];
  pipe( fd );

	pid_t pid = fork();
	if( pid < 0 ) return NULL;
  if( pid ){  // main process
    close( fd[1] ); // close out

    char *b = (char *)malloc( 1024 );
    //printf("exec %s pid %i\n", *argv, pid);

    FILE *f = fdopen( fd[0], "r" );
    while( 1 ){
      char *s = fgets( b, 1024, f );
      if( !s ) break;
      fn( b );
    };
    fclose(f);
    return 1;
  }

  // child process
  dup2( fd[1], 1 ); // redirect out
  dup2( fd[1], 2 ); // redirect out
  close( fd[0] ); // close in
  close( fd[1] ); // close out

	setsid() ;   /* become session leader */
	chdir( home ) ; /* change the working dir */
	umask( 0 ) ;   /* clear out the file mode creation mask */
	exit( execvp( *argv , (char * const *)argv ) ? EXIT_FAILURE : EXIT_SUCCESS );
}

int _exec( const char ** argv, int push, const char *dir ){
	//static const char * home = getenv("HOME");
  if( !argv || !*argv ) return -1;
	pid_t pid = fork();
	if( pid < 0 ) return -1;
  if( pid ){  // main process
    //printf("exec %s  dir %s pid %i\n", argv[0], dir, pid);
    if( push ) pushPid( pid );
    return 1;
  }

  // child process
	setsid() ;   /* become session leader */
	chdir( dir ? dir : home ) ; /* change the working dir */
	umask( 0 ) ;   /* clear out the file mode creation mask */
	exit( execvp( *argv , (char * const *)argv ) ? EXIT_FAILURE : EXIT_SUCCESS );
}
