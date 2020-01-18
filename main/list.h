
#ifndef _gdllist_
#define _gdllist_

#include "../Gdl/Gdl.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define LIST_MIN_UPDATE_TIME 10000

struct entrie {
	const char * label;
	const char * comment;
//	char * data; // user data
	u32 data;
	u8 type;
	clDeep * icon;
};

//char path[256];

struct list {
	//char path[256];
	int number;
	int inumber;
	int selected;
	int last;
	int isScrollBar;
	u32 lines;
	struct entrie * entries;
	u32 scrollRatio;
	u32 scrollFill;
	u32 scrollPos;
	u32 lastNoKey;
	u32 lastNoKeyTime;
	u32 lastMove;
	u32 lastUpdate;
	u32 moveTime;
	char * labels;
};

struct entrieCommand {
	const char * label;
	int(*fn)();
};

struct list * newList( int number, int selected );
void drawList( struct list * l );
void freeList( struct list * l );
int listCheckKey( struct list * l );
struct list * string2List( const char ** s );
struct list * entrieCommands2List( struct entrieCommand * cmds );
int selectFromList( const char * title, struct list * l );
int showEntrieCommandsList( const char * title, struct entrieCommand * e, struct list * l );
void refreshList( struct list * l );
struct entrie * appendList( struct list * l, int nb );
char * askUser( const char * title );
void logList( struct list * l );

#endif
