
#ifndef _apt_
#define _apt_

#include "../Gdl/Gdl.h"

struct package {
	char * version;
	char * description;
	char * name;
	const char * status;
};

struct repo {
	struct package ** packages;
	char * b;
	int nb;
};

struct server {
  struct repo * installed;
  struct repo * available;
  struct repo * upgradable;
};

struct server * aptInit( void );

#endif
