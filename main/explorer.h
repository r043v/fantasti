
#ifndef _gdlexplorer_
#define _gdlexplorer_

#include "../Gdl/Gdl.h"
#include "list.h"
#include "pid.h"

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <elf.h>
#include <zip.h>

extern void drawBg(void);

#define PATH_BUFFER 2048
#define READ_DIR_NB_BLOC (1024*16)
#define MAX_PATH_PART 64

struct pathEntry {
	struct list * list;
	char * folder;
};

struct explorer {
	char bf[PATH_BUFFER], *b, *path, *p;
	struct pathEntry entries[MAX_PATH_PART];
	u32 length;
};

/*struct explorer {
  struct path path;
}*/

struct explorer * explorer( const char * );
void showExplorer( struct explorer * );
void freeExplorer( struct explorer * );

#endif
