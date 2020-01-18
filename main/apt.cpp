
#include "./apt.h"

extern clDeep **greenfont, **bluefont;
extern int askUser(const char * msg, const char **options, int nb, int selected);

char * cmd( const char *cmd, u32 * length ){
	char *bf = (char*)malloc(256*1024), *b = bf;
	FILE* c = popen( cmd, "r" );

  setvbuf ( c , NULL , _IOLBF , 1024 );

	while(1){
		size_t n = fread( b, 1, 32, c );
		b += n;
		if( n != 32 ) break;
	};

	*b++ = 0;
	*length = b - bf;

  if( !*length ){
    free(bf); return 0;
  }

	return (char*)realloc(bf, *length );
}

void lcmd( const char *cmd ){
	char bf[16];
	FILE* c = popen( cmd, "r" );
  setvbuf ( c , NULL , _IOLBF , 1024 );
	while( fgets(bf, sizeof(bf), c) != NULL){
		shell.print( bf );
	};
}

const char * repoFile = "/var/lib/apt/lists/gs.dread.fr_dists_stretch_main_binary-armhf_Packages";
const char * updateCmd = "sudo apt-get -qq update -o Dir::Etc::sourcelist=/etc/apt/sources.list.d/gs.list";
const char * listCmd = "apt-cache dumpavail -o Dir::Etc::sourcelist=/etc/apt/sources.list.d/gs.list";
const char * aptgetInstall = "unbuffer sudo apt-get -qq -y install %s";
const char * aptgetReinstall = "unbuffer sudo apt-get -qq -y --reinstall install %s";
const char * aptgetRemove = "unbuffer sudo apt-get -qq -y remove %s";
const char * yesno[2] = { "yes", "no" };
const char * installString[3] = { "install ?", "upgrade ?", "reinstall ?" };

void repoDump( struct repo * r ){
	for( int no = 0; no < r->nb; ){
		struct package *p = r->packages[no];
		shell.print("%i/%i : %s (%s)\n", ++no, r->nb, p->name, p->version );
	}
}

void repoDiff( struct repo * local, struct repo * online ){
	repoDump( online );
	repoDump( local );
	shell.print("- - - - -");
}

void parseWord( struct repo * repo, char * word, char * data ){
	struct package * package;

	if( !strcmp( word, "Package" ) ){
		package = (struct package *)malloc(sizeof(struct package));
		memset(package,0,sizeof(struct package));
		repo->packages[ repo->nb++ ] = package;
		package->name = data;
		return;
	} else package = repo->packages[ repo->nb - 1 ];

	if( !strcmp( word, "Description" ) ){
		package->description = data;
		/*shell.print("\n%s %s\n%s\n",
			package->name,
			package->version,
			package->description
		);*/
		return;
	}

	if( !strcmp( word, "Version" ) ){
		package->version = data;
		//shell.print("v %s\n",package->version);
		return;
	}

	if( !strcmp( word, "Status" ) ){
		package->status = data;
		//shell.print( "%s", data );
		return;
	}
}

struct repo * createRepo(){
	struct repo * r = (struct repo *)malloc( sizeof(struct repo) );
	r->packages = (struct package **)malloc( 256 * sizeof(struct package*) );
	r->nb = 0;
	return r;
}

void freeRepo( struct repo * repo ){
  free( repo->packages );
  free( repo );
}

struct repo * readRepo( u8 *bf, u32 length, struct repo * r ){
	if(!r) r = createRepo();

	u8 *p = bf, *end = &p[ length - 1 ], *word = p, *data = 0;
	r->b = (char*)malloc(1024*1024);
	char *b = r->b;

	while( p <= end ){
		while( p != end && (*p == ' ' || *p == '\n') ) p++;
		if( p == end ) break;
		word = p;
		while( p != end && *p != ':' ) p++;
		if( p == end ) break;
		*p++ = 0; // end of word
		//shell.print( "%s ", word );
		while( p != end && *p != ' ' ) p++;
		if( p == end ) break;
		while( p != end && (*p == ' ' || *p == '\n') ) p++;
		if( p == end ) break;
		data = p;
		while( p != end && *p != '\n' ) p++;
		if( p == end ) break;
		*p++ = 0; // end of data
		u32 l = p - data;
		memcpy(b,data,l);
		//shell.print("***%s***\n---%s---\n", word, b );
		parseWord( r, (char*)word, b );
		b += l;
		if( p == end ) break; // ?
		while( p != end && *p == '\n' ) p++;
		if( p == end ) break;
	};

	r->b = (char*)realloc( r->b, r->b - b );

	return r;
}

void reboot( void ){
	shell.print(" * press a key to reboot *");
	while(1){
		Gdl_flip();

		if( keyUp(ka)
			|| keyUp(kb)
			|| keyUp(kx)
			|| keyUp(ky)
			|| keyUp(kstart)
			|| keyUp(kselect)
		) exit(1);//Gdl_exit();
	};
}


//struct repo * repos[ 3 ];
/*int selectedX = 0;
int selectedY = 0;


void drawMenu( void ){
	static struct repo * r = repos[ selectedX ];
	const char * lbl[3] = { "available", "upgrade", "installed" };

	if( keyUp(kleft) ){
		if(selectedX-- == 0) selectedX = 2;
		r = repos[ selectedX ];
		if( selectedY >= r->nb ) selectedY = r->nb - 1;
	}

	if( keyUp(kright) ){
		if(selectedX++ == 2) selectedX = 0;
		r = repos[ selectedX ];
		if( selectedY >= r->nb ) selectedY = r->nb - 1;
	}

	if( keyUp(kup) ){
		if(selectedY-- == 0) selectedY = r->nb - 1;
	}

	if( keyUp(kdown) ){
		if(++selectedY >= r->nb) selectedY = 0;
	}

	if( keyUp(kenter) )
		drawGdlShell = !drawGdlShell;

	if( !r->nb ) selectedY = 0;

	if( r->nb && keyUp(ka) ){
		if( askUser( installString[selectedX], yesno, 2, 0 ) == 0 ){
			char bf[256];
			struct package *p = r->packages[ selectedY ];
			sprintf( bf, selectedX > 1 ? aptgetReinstall : aptgetInstall, p->name );
			shell.print("* run %s\n", bf);
			drawGdlShell = true;
			Gdl_flip();
			lcmd(bf);
			reboot();
		}
	}

	if( selectedX > 0 && r->nb && keyUp(ky) ){
		struct package *p = r->packages[ selectedY ];
		if( strcmp("gs-pingu", p->name) && askUser( "remove ?", yesno, 2, 1 ) == 0 ){
			char bf[256];
			sprintf( bf, aptgetRemove, p->name );
			shell.print("* run %s\n", bf);
			drawGdlShell = true;
			Gdl_flip();
			lcmd(bf);
			reboot();
		}
	}

	int n = 0, x=36;

	for( n = 0; n < 3; n++ ){
		setGdlfont( n == selectedX ? greenfont : bluefont );
		prints(x,24,"%s", lbl[n] );
		x += 96;
	};

	if( r->nb ){
		for( n = 0; n < r->nb; n++ ){
			struct package *p = r->packages[ n ];
			//shell.print("%i/%i : %s (%s)\n", ++no, r->nb, p->name, p->version );
			setGdlfont( n == selectedY ? greenfont : bluefont );
			prints(24,42+12*n,"%i : %s (%s)", n, p->name, p->version );
		}

		struct package *p = r->packages[ selectedY ];

		setGdlfont( greenfont );

		prints(24,180,"version %s", p->version );

		if( p->status )
			prints(28,190,"%s", p->status );

		prints(24,200,"%s", p->description );

	}	else prints(32,40,"empty .." );

		setGdlfont( greenfont );
}*/

struct server * aptInit( void ) {
//	shell.print(" - pingu ! - an apt manager\n - by r- for game Jam #1\n\nwe don't need games, we need apt\n\n");	Gdl_flip();
//	shell.print("* run apt-get update..\n"); Gdl_flip();

	u32 length = 0;
	char *res = cmd( updateCmd, &length ); if(res) free(res);

	length = 0;
	u8 *repo = loadFile(repoFile, &length);

	if( !length ){
//		shell.print("local repo data not found.\n");
    return 0;
  }

//	shell.print("check availables packages\n");
	struct repo *available = createRepo();
	readRepo( repo, length, available ); free(repo);

//	shell.print("gs repo got %i packages\n", available->nb);

	//char ** packages = (char**)malloc( (available->nb + 1) * sizeof(char*) );
	char * bf = (char*)malloc(256);

	struct repo * toInstall = createRepo();
	struct repo * toUpdate = createRepo();
	struct repo * installed = createRepo();

	for( int n = 0; n < available->nb; n++ ){
		struct package *p = available->packages[n];
		sprintf(bf,"dpkg -s %s",p->name);
		length = 0;
		char * dpkg = cmd(bf,&length);

		if( length > 1 ){ // found on local
      struct repo * local = createRepo();
			readRepo( (u8*)dpkg, length, local );
			struct package *lp = local->packages[ local->nb - 1 ];
			//shell.print("%s %s\n",lp->name,lp->status);
			p->status = lp->status;
			// check version
			if( strcmp(lp->version,p->version) != 0 ){
				// not same version, can be upgraded
				// push this package to update list
				toUpdate->packages[ toUpdate->nb++ ] = p;
			} else {
				// version is up to date
				// push this package in installed list
				installed->packages[ installed->nb++ ] = p;
			}

      freeRepo( local );
		} else {
//			shell.print("%s not installed\n",p->name);
			p->status = "not installed";
			// push this package to install list
			toInstall->packages[ toInstall->nb++ ] = p;
		}

    free( dpkg ); // dpkg return
	}

	free(bf);
  freeRepo( available );

  //struct repo ** repos = (struct repo **)malloc( 3*sizeof(struct repo *) );//[ 3 ];
  struct server * server = (struct server *)malloc( sizeof(struct server) );

  server->installed = installed;
  server->available = toInstall;
  server->upgradable = toUpdate;
//  shell.print("%u available\n%u installed\n%u upgradable\n",toInstall->nb,installed->nb,toUpdate->nb);

  return server;
}
