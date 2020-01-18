
#include "list.h"

	void logList( struct list * l ){
		for( u32 n=0; n < l->number ; n++ ){
			printf("%u %s\n",n, l->entries[n].label );
		};
	}

	void drawList( struct list * l ){
		int entrie = 0;

		if( l->isScrollBar ){
			entrie = l->selected - 10 - ( l->selected & 1 );
			if( entrie < 0 ) entrie = 0;
			else if( entrie + 20 > l->number ) entrie = l->number - 20 + ( l->number & 1 );
		}

		if( l->isScrollBar ){
				l->scrollPos = 15 + ( (( (entrie >> 1) + (entrie & 1) ) * l->scrollRatio)>>10 );
				clDeep * s = xy2scr( (SCREEN_WIDTH-2), l->scrollPos );
				u32 n = l->scrollFill;
				while(n--){
					*s = 0xff00ff;
					s[1] = 0xff00ff;
					s += SCREEN_WIDTH;
				};
		}

		struct entrie * e = &l->entries[entrie];

		for(u32 cpt=0;cpt<20;cpt++)
		{	if( entrie == l->number ) break ;

			u32 c2 = cpt >> 1;
			u32 x = 5 + 160 * ( cpt&1 );
			u32 y = ( ( c2 + 1 ) * 16 ) + c2 + 10;
      //int xlimit = x+155;

			setBlitLimit( x, x+155 );

      if( !e->type ) // file
      {			if( entrie == l->selected ){
              //printf("<%s>\n",e->label);

//              printFreetype(defaultFreetypeFont,(u8*)e->label,x,y+defaultFreetypeFont->height);

      				e->label ?
      					fprints( x, y, ">%s", e->label )
      				:	fprints( x, y, ">%u", entrie )
      				;
      			} else {
      				e->label ?
      					fprints( x, y, " %s", e->label )
      				:	fprints( x, y, " %u", entrie )
      				;
      			}
      } else {
        if( entrie == l->selected ){
        				e->label ?
        					fprints( x, y, ">[%s]", e->label )
        				:	fprints( x, y, ">[%u]", entrie )
        				;
        			} else {
        				e->label ?
        					fprints( x, y, " [%s]", e->label )
        				:	fprints( x, y, " [%u]", entrie )
        				;
        			}
      }
			entrie++ ; e++;
		}

		fullBlitLimit();
	}

	#define LIST_BLOC_LENGTH 8

	struct entrie * appendList( struct list * l, int nb ){
		int number = l->number + nb;

		//printf("append list from %i to %i (internal %u)\n", l->number, number,l->inumber);

		if( number > l->inumber ){ // current allocated memory is too small, realloc to add LIST_BLOC_LENGTH reserved entry
			int inumber = number + LIST_BLOC_LENGTH;
			//printf("list realloc to %u\n",inumber);
			struct entrie * e = (struct entrie *)realloc( l->entries, inumber * sizeof(struct entrie) );
			if( !e ) return NULL; // out of memory, failed
			l->entries = e;
			memset( &l->entries[ l->inumber ], 0, ( inumber - l->inumber ) * sizeof( struct entrie ) );
			l->inumber = inumber;
		}

		struct entrie * out = &l->entries[ l->number ]; // will return first new entrie
		l->number = number; // update new list length

		refreshList( l ); // recompute internal properties

		return out;
	}

	void refreshList( struct list * l ){
		l->lines = (l->number >> 1) + (l->number & 1);

		l->isScrollBar = l->lines > 10;

		if( l->isScrollBar ){
			l->scrollRatio = (186*1024) / l->lines;
			l->scrollFill = ( 10 * l->scrollRatio ) >> 10;
		}
	}

	struct list * newList( int number, int selected ){
		struct list * l = (struct list *)malloc( sizeof(struct list) );

		l->inumber = number + LIST_BLOC_LENGTH; // internal number, for future list expand

		l->entries = (struct entrie *)malloc( l->inumber * sizeof(struct entrie) );
		//l->icons = (clDeep**)malloc( number * sizeof(clDeep*) );
		l->labels = 0;
		l->number = number;
		l->selected = selected;
/*		l->lines = (l->number >> 1) + (l->number & 1);

		l->isScrollBar = l->lines > 10;

		if( l->isScrollBar ){
			l->scrollRatio = (186*1024) / l->lines;
			l->scrollFill = ( 10 * l->scrollRatio ) >> 10;
		}
*/

		refreshList( l );

		struct entrie *e = l->entries, *last = &l->entries[l->number];
		while( e < last ){ e->label = 0; e++; };

		l->last = 0;
		l->lastNoKey = 0;
		l->lastNoKeyTime = 0;
		l->lastMove = 0;
		l->moveTime = 0;

		return l;
	}

	struct list * list( const char ** labels, int number ){
		struct list * l = newList( number, 0 );
		struct entrie *e = l->entries, *last = &l->entries[l->number];
//		char * const * label = labels;
		while( e < last ){
			e->label = *labels++; e++;
		};

		return l;
	}

	void freeList( struct list * l ){
		if( l->labels ) free( l->labels );
		free( l->entries );
		free( l );
	}

extern int needUpdate;

	int listCheckKey( struct list * l ){

		int update = needUpdate; needUpdate = 0;

    //printf("key %u\n", isKeyPress);
		if(! isKeyPress ){
			l->lastNoKey = l->lastMove = tick;

			if( tick - l->lastUpdate > LIST_MIN_UPDATE_TIME ){
				l->lastUpdate = tick;
				return true; // force refresh
			}

			return update;
		}

		l->lastNoKeyTime = tick - l->lastNoKey;

    if( l->lastNoKeyTime > 2000 ) l->moveTime = 8;
    else {
      if(l->lastNoKeyTime > 1500) l->moveTime = 12;
		  else {
			  if(l->lastNoKeyTime > 1000) l->moveTime = 42;
			  else {
				  if(l->lastNoKeyTime > 500)	l->moveTime = 102;
				  else	l->moveTime = 192;
			  }
		  }
    }

		if( l->selected >= l->number ){
			l->selected = l->number ? 0 : -1;
		}

		l->last = l->selected;

		if( tick - l->lastMove > l->moveTime ){
			if(keyPush(kup)) l->selected -= 2 ;
			else if(keyPush(kdown))	l->selected += 2 ;
		} else {
			if(keyUp(kup)) l->selected -= 2 ;
			else if(keyUp(kdown)) l->selected += 2 ;
		}

		if(keyUp(kleft)){
			/*if( l->selected & 1 ) l->selected-- ;
			else {

			}*/
			if( l->selected ) l->selected--;
		} else {
			if( keyUp(kright) )
				/*if( !( l->selected & 1 )) l->selected++ ;
				else {

				}*/
				if( l->selected < l->number-1 ) l->selected++;
		}

		if( l->last != l->selected ){
			l->lastMove = tick;
      //l->lastNoKey = tick;
			if( l->selected < 0 )
				l->selected = abs( l->selected % 2 ) ;
			else{
				if( l->selected > l->number-1 )
					l->selected -= 2 ;
			}

			l->lastUpdate = tick;

			return true;
		}

		if( tick - l->lastUpdate > LIST_MIN_UPDATE_TIME ){
			l->lastUpdate = tick;
			return true; // force refresh
		}

		return update;
	}

	struct list * string2List( const char ** s ){
		u32 nb = 0;
		while( s[ nb++ ] );
		nb --;
		struct list * l = newList( nb, 0 );
		struct entrie *e = l->entries, *last = &l->entries[l->number];

		u32 n = 0;

		while( e < last ){
			e->label = s[ n++ ];
			e->type = 0;
			e++;
		};

		return l;
	}

struct list * entrieCommands2List( struct entrieCommand * cmds ){
	u32 nb = 0;
	while( cmds[ nb++ ].label != NULL );
	nb --;

	struct list * l = newList( nb, 0 );
	struct entrie *e = l->entries, *last = &l->entries[l->number];

	u32 n = 0;

	while( e < last ){
		e->label = cmds[ n++ ].label;
		e->type = 0;
		e++;
	};

	return l;
}

  int showEntrieCommandsList( const char * title, struct entrieCommand * e, struct list * l ){
  	while( 1 ){
			int n = selectFromList( title, l );
			if( n == -1 ) return -1 ; // escape
			n = e[ n ].fn();
			if( n == -1 ) return 0 ;
		}
  }

  extern void drawBg(void);

  int selectFromList( const char * title, struct list * l ){
  	int update = true;
  	while(1) {  // main loop
  		update |= listCheckKey( l );

  		if( keyRelease(kenter) ) return l->selected;
  		if( keyRelease(kesc) ) return -1;

  		if( update ){
  			update = false;

  			drawBg();
  			drawList( l );
  			fprints(4,0,"%s",title);
				if( l->number )
					fprints(4,204,"%s", l->entries[ l->selected ].label );
//				else
//					fprints(4,204,"empty");
  			Gdl_flip(); // update window
  			continue;
  		}

  		Gdl_updateMsg();
  		tick = GetTickCount();
  		Gdl_Sleep(1);
  	};
  }

char * askUser( const char * title ){
  char * passList[ 256 ] = { "done", "<-", "space" };
  const char * symbol = "?!/<>()[]{}:;,=+'_-~&@%";
  char * passListBf = (char*)malloc(1024), *p = passListBf;
  u32 c, n = 3;
  const char * cp = symbol;
  while( *cp ){
    passList[ n++ ] = p;
    //printf("%c\n",*cp);
    *p++ = *cp++;
    *p++ = 0;
  };

  for( c = 'a' ; c <= 'z' ; c++ ){ passList[ n++ ] = p; *p++ = c; *p++ = 0; }
  for( c = 'A' ; c <= 'Z' ; c++ ){ passList[ n++ ] = p; *p++ = c; *p++ = 0; }
  for( c = '0' ; c <= '9' ; c++ ){ passList[ n++ ] = p; *p++ = c; *p++ = 0; }

  passList[ n ] = NULL;

  struct list * l = string2List( (const char **)passList );
  char * password = (char*)malloc(256), *ppassword = password;
	*password = 0;
  char titleBf[ 256 ];
  char * titlePtr = titleBf;//[ strlen( title ) ];
  while( *titlePtr++ = *title++ );
  titlePtr--;

  while( 1 ){
    strcpy( titlePtr, password );

    int v = selectFromList( titleBf, l );

    //printf("%i\n",v);

    // cancel
    if( v == -1 ){
      password[ 0 ] = 0;
      break;
    }

    if( !v ) break; // done

    if( v == 1 ){ // remove last char
      if( ppassword != password )
        *--ppassword = 0;
      continue;
    }

    if( v == 2 ){ // space
      *ppassword++ = ' ';
      *ppassword = 0;
      continue;
    }

    struct entrie * e = &l->entries[ v ];
    *ppassword++ = *e->label;
    *ppassword = 0;
    //printf("%c %s\n", *e->label, password);
  };

  if( *password ){
    printf( "password %s\n", password );
  }

  free( passListBf );
  return password;
}
