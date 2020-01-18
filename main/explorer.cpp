
#include "explorer.h"

//struct path path;

/*void iniPath( void ){
  path.path = &path.bf[ PATH_BUFFER - 1024];
	path.b = path.bf;
	*path.path = 0;
	path.p = path.path;
  path.length = 0;
}*/

u32 isBin( const char* file ){
  FILE * f = fopen( file, "rb" );
  if( !f ) return 0;
  Elf32_Ehdr header;
  fread( &header, 1, sizeof(header), f );
  fclose( f );
  return ( memcmp(header.e_ident, ELFMAG, SELFMAG) == 0 );
}

	int direntComparator(const void *a, const void *b){
			const dirent * aa = (const dirent *)a;//*(const dirent **)a;
			const dirent * bb = (const dirent *)b;//*(const dirent **)b;

			// directory first ...
			if( aa->d_type == DT_DIR ){
				if( bb->d_type != DT_DIR ) return -1;
			} else {
				if( bb->d_type == DT_DIR ) return 1;
			}

			//return alphasort( &aa, &bb );
			return strcasecmp( aa->d_name, bb->d_name );
	}

	int filterPng( struct dirent * e ){
		const char * n = e->d_name;
		if( *n == '.' || !*n ) return false ; // skip hidden files && . or .. dir
		if( e->d_type == DT_DIR ) return true; // allow dir
		while( *n && *n != '.' ) n++; // jump to ext
		if( !*n++ ) return false; // need ext
		// only png
		if( *n != 'p' && *n != 'P' ) return false;
		n++;
		if( *n != 'n' && *n != 'N' ) return false;
		n++;
		if( *n != 'g' && *n != 'G' ) return false;
		n++;
		if( *n ) return false;
//		if( !strstr( n, ".png" ) ) re && !strstr( n, ".PNG" ) ) return false; // only png
		return true;
	}

	int filter( struct dirent * e ){
    if( e == NULL ) return false;
		const char * n = e->d_name;
		if( !*n || *n == '.' ) return false ; // skip hidden files && . or .. dir

//		if( e->d_type == DT_DIR ) return true; // allow dir

//    char *dot = strrchr((char*)n, '.');
//    if( dot && !strcmp(dot, ".srm") ) return false; // no retroarch save

		return true;
	}

	struct list * readDir( const char *path, int filter( struct dirent *) ){
		DIR *dir;
		if (!(dir = opendir(path))){
      printf("cannot read dir %s\n",path);
      return 0;
    }

		u32 nb = 0, max = READ_DIR_NB_BLOC, labelsLength = 0;
		struct dirent *p = 0, *entries = (struct dirent *)malloc( max * sizeof(struct dirent) );

    if( !entries ){
      printf("alloc error\n");
    }

		struct dirent *entry;
		while( (entry = readdir(dir)) != NULL ){
			//if( *entry->d_name == '.' ) continue; // skip hidden files && . or .. dir
			if( !filter(entry) ) continue;
			//printf( "%s\n", entry->d_name );
			labelsLength += strlen( entry->d_name ) + 1;

			//entries[ nb++ ] = entry;
      memcpy( &entries[nb++], entry, sizeof(struct dirent) );
			if( nb == max ){
				max += READ_DIR_NB_BLOC;
        printf("reach max files, realloc\n");
				p = (struct dirent *)realloc( entries, max * sizeof(struct dirent) );
        if( p ) entries = p; else printf("realloc error\n");
			}
		};

    if( !nb ){
      printf("no entry into %s folder\n",path);
      closedir( dir );
  		free( entries );
      return 0;
    }

    p = (struct dirent *)realloc( entries, nb * sizeof(struct dirent) );
		if( p ) entries = p; else printf("realloc error\n");

		qsort( entries, nb, sizeof( struct dirent /* * */ ), direntComparator );
//		printf("qsort on %u!\n",nb);

		//for( u32 n = 0; n<nb; n++ ) printf("%u %s\n",n,entries[n]->d_name);

		struct list * l = newList( nb, 0 );
		struct entrie *e = l->entries, *last = &l->entries[ l->number ];
		l->labels = (char*)malloc( labelsLength );
//    printf( "labelsLength %u for %u entries\n", labelsLength, l->number );
    if( !l->labels ) printf("alloc error\n");
		char *label = l->labels;
		//struct dirent *d = entries;
    u32 cpt = 0;
		while( e < last ){
      struct dirent *d = &entries[ cpt ];
			//struct dirent *dd = *d++;
			e->label = label;
			char * n = d->d_name;
			while( *n ) *label++ = *n++;
      *label++ = 0;
			e->type = d->d_type == DT_DIR; // is folder
//      if( !strlen(e->label) )
//        printf("%u] %u %s %s\n", cpt, e->type, e->label, d->d_name);
      //d++;
      cpt++;
			e++;
		};

//		printf("close\n");
		closedir(dir);
		free( entries );

		return l;
	}

void pathBack( struct explorer * x ){
//  printf("enter path back length %u, path %s\n",x->length, x->path);
	if( x->length < 2 ) return;
  //printf("check entrie %u\n", x->length);
	struct pathEntry * e = &x->entries[ --x->length ]; // current entry
//  printf("entry %u [%s] list %p\n", x->length, e->folder, e->list);
  if( e->list ){ freeList( e->list ); e->list = 0; }
//  u32 l = x->b - e->folder; // last entry length
	x->p -= ( x->b - e->folder );
//  printf("new p [%s]\n",x->p);
  *x->p = 0;
  x->b = e->folder;
  *x->b = 0;
//	printf( "back to %s length %u\n", x->path, x->length );
}

void pathPush( struct explorer * x, const char * name ){
	struct pathEntry * e = &x->entries[ x->length++ ];
	const char * n = name;
	e->folder = x->b; // entrie name
	while( ( *x->b++ = *x->p++ = *n++ ) ); // add into folder & full path
  x->p--;
  *x->p++ = '/';
  *x->p = 0;
  //*x->b = 0;
	//printf("append %s to path %u %s\n", name, path.length, path.path );
	e->list = readDir( x->path, filter );

  if( !e->list ){
    // directory is empty
    pathBack( x );
    return;
  }
	//printf("p %p\nb %p\n", (void*)path.p, (void*)path.b);
	//printf( "=> %s length %u\n", x->path, x->length );
}

void freeExplorer( struct explorer * x ){
  struct pathEntry * e = x->entries;
  for( u32 n = 0; n < x->length; n++ ){
    if( e->list ){ freeList( e->list ); e->list = 0; }
    e++;
  }
  free(x);
}

/**/

struct explorer * explorer( const char * path ){
  struct explorer * x = (struct explorer *)malloc( sizeof( struct explorer ) );

  x->path = &x->bf[ PATH_BUFFER - 1024];
	x->b = x->bf;
	*x->path = 0;
	x->p = x->path;
  x->length = 0;

  pathPush( x, path );

  return x;
}

#define ZIP_UNCOMPRESS_BUFFER (1024*1024)

void zipUnzip( zip *z, struct zip_stat *s ){
  //printf("zip unzip\n");
  if( !z || !s || !( s->valid & ( ZIP_STAT_NAME | ZIP_STAT_INDEX | ZIP_STAT_SIZE ) ) ) return;
  //printf("zip open\n");
  zip_file *f = zip_fopen_index( z, s->index, 0 );
  if( !f ) return;

  char *bf = (char *)malloc( ZIP_UNCOMPRESS_BUFFER + 2048 ), *path = bf + ZIP_UNCOMPRESS_BUFFER;
  if( !bf ){
    printf("malloc error\n");
    zip_fclose(f);
    return;
  }

  const char *name = s->name;
  {
    const char *namePtr = name;
    while( *namePtr ){
      if( *namePtr++ == '/' ) name = namePtr;
    };
  }

  sprintf( path, "/tmp/%s", name );
  //printf( "%s\n",path );
  FILE * out = fopen( path, "w" );

//  printf("read file of %li bytes\n", s->size);

  zip_int64_t l = 0, t = 0;
  while(1){
    l = zip_fread( f, bf, ZIP_UNCOMPRESS_BUFFER );
    if( l < 1 ) break;
    //printf( "pipe %li bytes\n",l );
    t += l;
    if( 1 != fwrite( bf, l, 1, out ) ) break;
  };

  //printf("writed %li/%li bytes to %s\n",t,s->size,name);

  zip_fclose(f);
  fclose(out);

  //printf("exec %s\n",path);
  //const char * cmds[] = { "xdg-open", path, NULL };
  const char * cmds[] = { "mimeopen", "--no-ask", path, NULL };
  _exec( cmds );

  free(bf);
}

void zipExplore( const char * file, const char * startPath = "" ){
  int err = 0;
  zip *z = zip_open( file, ZIP_RDONLY, &err);
  if( !z ) return;
  zip_int64_t nb = zip_get_num_entries( z, 0 );
  //printf("%s got %li files\n",file, nb);
  if( !nb ){ zip_close(z); return; }

  char * path = (char*)malloc( 2048 ), *pathPtr = path;//, *title = path + 1536;
  // copy path
  { const char * p = startPath;
    while( *p ) *pathPtr++ = *p++ ;
    *pathPtr = 0;
  }

  const char * fileName = file;
  {
    const char *namePtr = fileName;
    while( *namePtr ){
      if( *namePtr++ == '/' ) fileName = namePtr;
    };
  }

  u32 pathLength = pathPtr - path;
  char * startPathEnd = pathPtr;
  //printf( "path %s length %u\n", path, pathLength );

  // retreve files info
  struct zip_stat *stats = (struct zip_stat *)malloc( nb * sizeof( struct zip_stat ) ), *stat = stats, *firstFile = 0 ;

  u32 foldersNb = 0, filesNb = 0;
  for( zip_int64_t n = 0; n < nb; n++ ){
    zip_stat_init( stat );
    if( err = zip_stat_index( z, n, 0, stat ) ) continue;
    if( !( stat->valid & (ZIP_STAT_NAME | ZIP_STAT_SIZE) ) ) continue;

    // check file name start with the start path
    if( pathLength ){
      if( strncmp( startPath, stat->name, pathLength) // not into this folder
      ||  !stat->name[ pathLength ] // the folder himself
      ){
        //printf("trim %s\n",stat->name);
        continue;
      }
    }

    if( stat->size ){
      //printf( "file %s %li ok %i\n", stat->name, stat->size, strncmp( startPath, stat->name, startPathLength) );
      filesNb++;
      if( !firstFile ) firstFile = stat;
    } else {
      //printf( "folder %s\n", stat->name );
      foldersNb++ ;
    }

    stat++;
  }

  if( filesNb == 1 ){
    //printf("single file\n");
    zipUnzip( z, firstFile ); // path path ?
    free( stats );
    zip_close( z );
    return;
  }

  u32 entriesNb = filesNb + foldersNb;

  // realloc stats
  if( firstFile = (struct zip_stat *)realloc( stats, entriesNb * sizeof( struct zip_stat ) ) )
    stats = firstFile;

  while(1){
    struct list * l = newList( 0, 0 );
    for( u32 n = 0; n < entriesNb; n++ ){
      struct zip_stat * stat = &stats[n];
      if( strncmp( path, stat->name, pathLength) ) continue;
      const char * name = stat->name + pathLength;
      if( !*name ) continue; // folder himself
      // search for '/'
      const char * p = name;

      while( *p && *p != '/' ) p++;
      bool isFolder = *p == '/';
      if( isFolder && p[1] ) continue; // file from other sub folder
      struct entrie * e = appendList( l, 1 );
      e->label = name;
      e->type = isFolder ? 1 : 0;
      e->data = n;
      //printf("%s %s %i\n",stat->name, name, isFolder);
    }

    //logList( l );

    // empty list ?
//    if( !l->number ){
      // TODO path back
//    }

/* select from list */

  	int update = true, selected = -1;
  	while(1) { // draw list
  		update |= listCheckKey( l );

      /* auto enter single folder */
  		if( (l->number == 1 && l->entries[0].type == 1) || keyRelease( kenter ) ){
        selected = l->selected;
        struct entrie * e = &l->entries[ selected ];

        // is a folder ?
        if( e->type == 1 ){
          // path push
          const char * p = e->label;
          while( *p ) *pathPtr++ = *p++ ;
          *pathPtr = 0;
          pathLength = pathPtr - path;
//          printf("=> path %s\n",path);
          break;
        }

        // is a file, unzip && exec
        zipUnzip( z, &stats[ e->data ] );
      }

  		if( keyRelease( kesc ) ){
        // exit zip
        free( stats );
        free( path );
        zip_close( z );
        return;
      }

      if( ( !l->number ) || keyRelease( kspace ) ){
        // path back
        char * p = pathPtr;
        if( p != startPathEnd ){
          p--; // we are in subfolder, skip last /
          do p--; while( p != startPathEnd && *p != '/' );
          *p = 0;
          if( p == startPathEnd ){
            //*p = 0; // reach start
            //printf("back to start path\n");
          }
          else {
            //*++p = 0;
          }

          pathPtr = p;
          pathLength = pathPtr - path;
          break;
        }

        free( stats );
        free( path );
        zip_close( z );
        return;
      }

  		if( update ){
  			update = false;

  			drawBg();
  			drawList( l );

        fprints(4,204,"%s", l->entries[ l->selected ].label );
        fprints(4,0,"%s/%s %u/%u",fileName,path,l->selected+1,l->number);

        Gdl_flip(); // update window
  			continue;
  		}

  		Gdl_updateMsg();
  		tick = GetTickCount();
  		Gdl_Sleep(1);
  	};

    freeList(l);
  };

  free( path );
  free( stats );

  zip_close(z);
}

void showExplorer( struct explorer * x ){
  struct list * l = x->entries[ x->length - 1 ].list;

  	int update = true;
  	do {  // main loop
  		//fps = countFps();

  		update |= listCheckKey( l );

  		if( keyUp(kenter) ){
  			struct entrie * e = &l->entries[ l->selected ];
  			if( e->type ){ // folder
  				//printf("%s\n", e->label );
  				pathPush( x, e->label );
  				l = x->entries[ x->length - 1 ].list;
  				//drawGdlShell ^= 1;
  				update = true;
  			} else { // file

          char * file = ( char * )malloc(4096);
          sprintf( file, "%s%s", x->path, e->label );
          bool checkBin = false;

          // search for dot to check ext
          const char * f = e->label, *ext = 0;

          while( *f ){ if( *f == '.' ) ext = f; f++; }
          if( !ext ) // not dot found, or dot is the first char, maybe bin ?
            checkBin = true;

          if( checkBin && isBin(file) )
            exec( file );
          else {
            //printf("file %s ext %s\n",file,f);
            if( !strcasecmp( ext+1, "zip" ) ){
              do { Gdl_updateMsg(); tick = GetTickCount(); Gdl_Sleep(1); } while( keyPush(kenter) );
              zipExplore(file);
              update = true;
            } else {
              const char * path = file;
              char * fileName = (char*)file;
              // lol
              {
                char *fPtr = fileName;
                while( *fPtr ){
                  if( *fPtr++ == '/' ) fileName = fPtr;
                };
              }
              fileName--;
              *fileName++ = 0;

              //const char * cmds[] = { "xdg-open", fileName, NULL };
              const char * cmds[] = { "mimeopen", "--no-ask", fileName, NULL };
              _exec( cmds, 1, path );
            }
          }

          free( file );
  			}
  		}

      if( keyRelease( kesc ) ){
        return;
      }

      if( keyRelease( kspace ) ){
        //printf("kspace\n");
        if( x->length > 1 ){ //&& keyRelease( kspace ) ){
          //printf("request back\n");

            //printf("path back %u %s\n",x->length,x->path);
    				pathBack( x );
            //printf("= %u %s\n",x->length,x->path);
    				l = x->entries[ x->length - 1 ].list;
    				update = true;
    		}
      }

  		if( update ){
  			update = false;

  			drawBg();
  			drawList( l );
//  			fprints(4,0,"%s %u %u | %u %u %u",x->path,l->selected,x->length, tick, l->lastNoKey, tick - l->lastNoKey);

        fprints(4,204,"%s", l->entries[ l->selected ].label );

        fprints(4,0,"%s %u/%u",x->path,l->selected+1,l->number);

        //struct entrie * e = &l->entries[ l->selected ];
        //printf("%s\n", e->label );

  			Gdl_flip(); // update window
  		} else { Gdl_updateMsg(); tick = GetTickCount(); Gdl_Sleep(1); }

    } while(1);
}
