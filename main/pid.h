
#ifndef _gdlpid_
#define _gdlpid_

#include "../Gdl/Gdl.h"
#include "list.h"
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

//void initPidStack( void );
void pushPid( pid_t pid );
void refreshPidStack( void );
struct list * pidStack2list( void );
void exec( const char * cmd, int push = 1 );
int _exec( const char ** argv, int push = 1, const char * dir = 0 );
char * p_exec( const char ** argv );
char ** pipe_exec( const char ** argv );
int callback_exec( const char ** argv, void(*fn)(char*) );

#endif
