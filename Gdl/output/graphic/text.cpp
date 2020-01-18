#ifndef _gdlTxt_
#define _gdlTxt_

#include "../../Gdl.h"

//#ifdef useFreetype

#define FONT_LIMIT 0xffff

#define rDec 16
#define gDec 8
#define bDec 0
#define FONT_COLOR_MIN 64

  FT_Library freetypeLibrary;

  struct freetypeFont * defaultFreetypeFont = 0;

  void initFreetype( void ){
    FT_Init_FreeType( &freetypeLibrary );
  }

void printChar( FT_Bitmap * b ){
  u32 sx = b->width, sy = b->rows ;
  u8 *p = b->buffer, *pend = p + sx*sy, *lend = p ;

  while( p != pend ){
    lend += sx;
    while( p != lend ){
      printf("%c",*p ? *p > 127 ? '*' : 'o' : ' ');
      p++;
    };
    printf("\n");
  };
}

  void setFreetypeColor( struct freetypeFont * f, u8 r, u8 g, u8 b, u8 minGrayValue ){
    f->r = 0xff - r;
    f->g = 0xff - g;
    f->b = 0xff - b;
    f->minGrayValue = minGrayValue; // min possible value for color
  }

  /* UTF-32 charcode to bitmap glyph to Gfm */
  void loadFreetypeGlyph( struct freetypeFont * f, UChar32 c ){
    //printf("load glyph for char %u %c\n",c,c);
    FT_Bitmap * bf = f->bitmap;
    FT_Face * face = &f->face;
    FT_UInt cglyph = FT_Get_Char_Index( f->face, c );
    //printf("glyph %u\n",cglyph);

    if( !cglyph ){
      //printf("glyph not found\n");
      // try fallback
      if( f->fallback ){
        cglyph = FT_Get_Char_Index( f->fallback, c );
        if( cglyph ){ // found in fallback
          //printf("found in fallback\n");
          bf = &f->fallback->glyph->bitmap;
          face = &f->fallback;
        }
      }

      if( !cglyph ){ // fallback char
        //printf("use a default glyph\n");
        cglyph = FT_Get_Char_Index( f->face, '*' );
      }
    }

    FT_Load_Glyph( *face, cglyph, FT_LOAD_RENDER );

//    printChar(bf);

    struct glyph * glyph = (struct glyph *)malloc( sizeof(struct glyph) );

    f->font[ c ] = glyph;
    glyph->top = f->face->glyph->bitmap_top;
    glyph->left = f->face->glyph->bitmap_left;

//    printf("%i %i\n",f->face->glyph->bitmap_top,f->face->glyph->bitmap_left);
    // clear buffer

    memset( f->buffer, 0xff , f-> height * f-> height * sizeof( clDeep ) );

//    const clDeep color = 0xff00ff;
/*
    const u32 r = 222;//64;//0xff;
    const u32 g = 222;//128;
    const u32 b = 222;//0xff;
*/
    // copy bitmap to buffer, reverted on y way
      u32 sx = bf->width, sy = bf->rows ;
      u8 *p = bf->buffer, *pend = p + sx*sy, *lend = p ;

      // !TODO support palette
      clDeep * clp = f->buffer;
      clDeep * clpl = clp;
      while( p != pend ){
        lend += sx;
        while( p != lend ){
//          printf("%c", *p ? '*' : '.' );
//          if( *p ) printf("%02x ",*p); else printf("   ");

          if( *p ){
/*            u32 rr = ( ( r + *p ) << (rDec - 1) ) & ( 0xff << rDec );
            u32 gg = ( ( g + *p ) << (gDec - 1) ) & ( 0xff << gDec );
            u32 bb = ( ( b + *p ) << (bDec - 1) ) & ( 0xff << bDec );*/

            u32 v = *p;// 0xff - *p;
            if( v < f->minGrayValue ) v = f->minGrayValue;
            u32 rr = ( /*1*/0xff - ( (f->r * v) >> 8 ) ) & 0xff;// if( rr < 64 ) rr = 64;
            u32 gg = ( /*1*/0xff - ( (f->g * v) >> 8 ) ) & 0xff;// if( gg < 64 ) gg = 64;
            u32 bb = ( /*1*/0xff - ( (f->b * v) >> 8 ) ) & 0xff;// if( bb < 64 ) bb = 64;

            *clp = (rr<<rDec) | (gg<<gDec) | (bb<<bDec);

            //if( *p != 0xff )
            //  printf("\n0x%02x > 0x%02x 0x%02x 0x%02x > 0x%08x",*p, rr, gg, bb, *clp);
            clp++;

          } else {
            *clp++ = 0xffffffff;
          }

          p++;
        };
//        printf("\n");
        clpl += f->height;
        clp = clpl;
      };

      int csx = f->height, csy = f->height;
      clDeep trColor = 0xffffffff;
      clDeep * cropped = cropSprite( f->buffer,&csx,&csy,0,0,1,1) ;
//      printf("cropped ..\n");
      glyph->gfm = scanImg(cropped,csx,csy,&trColor) ;
//      printf("scanned ..\n");
/*
    clDeep * array = Gfm2array( f->font[ c ] );
    clDeep * gfm = f->font[ c ];
    writeFile( "./font", array, (gfm[2]&0xffff) * (gfm[2]>>16) * sizeof( clDeep ) );
    free(array);
*/
      free( cropped );
  }

  void loadFreetypeChar( struct freetypeFont * f, UChar32 c ){
    if( c > FONT_LIMIT || f->font[ c ] ) return;
    loadFreetypeGlyph( f, c );
  }

  /* ascii */
  void loadFreetypeChars( struct freetypeFont * f, char * s ){
    while( *s ){
      if( f->font[ (u32)*s ] ){ s++; continue; }
      loadFreetypeGlyph( f, *s );
      s++;
    };
  }

  /* utf-8 */
  void loadFreetypeChars( struct freetypeFont * f, u8 * s ){
    u8 * p = s;
    UChar32 c;
    u32 i = 0;
//    printf("loading..\n");
    while( *p++ );
    p--;
    u32 l = p - s;
//    printf("l %u\n",l);
    while( 1 ){
//      printf("search utf8 .. %u/%u\n",i,l);
      U8_NEXT( s, i, l, c );
//      printf("c %u %u/%u\n",c,i,l);
      if( !c || i > l ) break;
//      printf("ok\n");

      if( c >= FONT_LIMIT || f->font[ c ] ){ /*printf("already loaded\n");*/ /*s++;*/ continue; }
//      printf("try load glyph..\n");
      loadFreetypeGlyph( f, c );
//      s++;
    };
  }

  struct freetypeFont * loadFreetypeFont( u8 * font, u32 fontSize, u32 height ){
    struct freetypeFont * f = (struct freetypeFont *)malloc( sizeof( struct freetypeFont ) );
    f->font = (struct glyph **)calloc( FONT_LIMIT, sizeof(struct glyph*) );

    FT_New_Memory_Face( freetypeLibrary, font, fontSize, 0, &f->face );
    FT_Set_Pixel_Sizes( f->face, 0, height );
    FT_Select_Charmap( f->face , FT_ENCODING_UNICODE);
    f->bitmap = &f->face->glyph->bitmap;
    f->height = height;
    f->buffer = (clDeep*)malloc( height * height * sizeof( clDeep ) );
    f->fallback = NULL;
    return f;
  }

  void setFreetypeFont( struct freetypeFont * f ){
    if( !f ) return;
    defaultFreetypeFont = f;
    //printf("default freetypeFont defined\n");
  }

  void setFallbackFont( struct freetypeFont * f, const char * font ){
    //printf("declare fallback font %s\n",font);
    FT_New_Face( freetypeLibrary, font, 0, &f->fallback );
    FT_Set_Pixel_Sizes( f->fallback, 0, f->height );
    FT_Select_Charmap( f->fallback , FT_ENCODING_UNICODE);
  }

  struct freetypeFont * loadFreetypeFont( const char * font, u32 height ){
    struct freetypeFont * f = (struct freetypeFont *)malloc( sizeof( struct freetypeFont ) );
    f->font = (struct glyph **)calloc( FONT_LIMIT, sizeof(struct glyph *) );

    FT_New_Face( freetypeLibrary, font, 0, &f->face );
    FT_Set_Pixel_Sizes( f->face, 0, height );
    FT_Select_Charmap( f->face , FT_ENCODING_UNICODE);
    f->bitmap = &f->face->glyph->bitmap;
    f->height = height;
    f->buffer = (clDeep*)malloc( height * height * sizeof( clDeep ) );
    f->fallback = NULL;
    return f;
  }

  void releaseFreetypeFont( struct freetypeFont * f ){
    FT_Done_Face( f->face );
    for( u32 n=0; n<FONT_LIMIT; n++ ){
      struct glyph * g = f->font[n];
      if( g ){
        free( g->gfm );
        free( g );
      }
    }
    free( f->font );
    free( f->buffer );
    FT_Done_Face( f->face );
    free( f );
    f = 0;
  }

  void releaseFreetype( void ){
    FT_Done_FreeType( freetypeLibrary );
  }

  /* ascii */
/*  void printFreetype( struct freetypeFont * f, char * s, int x, int y ){
    while( *s ){
      u32 g = (u32)*s;
      struct glyph * glyph = f->font[ g ];
      //clDeep * c = glyph->gfm;
      if( !glyph ){  // no glyph
        loadFreetypeChar( f, g );
        glyph = f->font[ g ];
        if( !glyph ){
          printf("char %c not found\n",*s);
          s++; continue;
        }
      }
      //if( !c ){ s++; continue; } // no glyph
      drawGfm( glyph->gfm, x, y - glyph->top );
      x += ( glyph->gfm[2]>>16 ) + CHAR_PADDING ; // char width
      if( x > (int)WIDTH ) return ;
      s++;
    };
  }*/

extern int frmWidthEnd;

  /* utf8 */
  void printFreetype( struct freetypeFont * f, u8 * s, int x, int y ){
    u8 * p = s;
    UChar32 c;
    u32 i = 0;
    while( *p++ );
    p--;
    u32 l = p - s;
    //printf("[%s] %u %u\n",s, strlen((char*)s), l);
    while( 1 ){
//      printf("%c ",s[i]);
      u8 v = s[i];
      U8_NEXT( s, i, l, c );
      //printf("%u %u/%u\n",c,i,l);
      if( !c || i > l ) break;
      //clDeep * g = f->font[ c ];
      if( c >= FONT_LIMIT ) c=32;
      struct glyph * g = f->font[ c ];
      if( c != 32 ){ // space ?
      //  struct glyph * g = f->font[ c ];
        if( !g ){  // no glyph
          loadFreetypeChar( f, c );
          g = f->font[ c ];
          if( !g ){
            printf("glyph %u not found\n",c);
            s++; continue;
          } else {
//            printf("char <%c> %u, glyph %u top %i left %i\n", (char)v, v, c, g->top, g->left );
          }
        }

        int left = g->left;
        //if( left == 0xff ) left = 0;

        //if( x < 320 )
        drawGfm( g->gfm, x + left, y - g->top );
        //printf("> %u\n",c);
        //printf("%u %li\n",c,f->face->glyph->metrics.horiAdvance>>8);

        x += ( g->gfm[2]>>16 ) + left;// ( g->left || CHAR_PADDING ) ; // char width
      } else x += CHAR_SPACE;

      if( x > frmWidthEnd ) break;// { printf(" screen out %u",x); break ;}
    };
//    printf("\n");
  }

//#endif

char dbg[2048];	// a buffer for the debug string
clDeep ** gdlfont = 0 ;

void drawText(const char *txt,int x, int y, int align, clDeep **font, int start, int max){
  if(!txt)  return ; // empty string
  if(!font) font = gdlfont; // default font
  const u8 * t = (const u8*)txt;
	const u8 * p = t ;
	while( *p++ && t - p < max );
  p-=2; // count string size, max is 32
	while(*p == 0x20) p--;	// remove space at end..
	p++; int sz = p - t;
/*  #ifdef use16b
   int sx = (*font)[0];
  #else
   int sx = ((*font)[2])>>16;
  #endif*/
  //if(max) if( sz > max ) sz = max ;
  //if(align) { x-=sz*sx ; }

  p = t ;

//  while( x < -sx && sz ) { x += sx ; p++ ; sz-- ; };
  while(sz--){
    u32 c = *p++;
    if( c < start || c > 123 ){ // bad char
      //printf("out of bound char %c %i\n",c,c);
      c = '*'; // continue;
    }
    c -= start;
    if( !c ){ x += CHAR_SPACE; continue; } // space char

    clDeep * charFrm = font[ c ];
    u32 sx = charFrm[2]>>16; // char width

    drawGfm(charFrm,x,y) ;
    x += sx + CHAR_PADDING ;

    if(x > (int)WIDTH) return ;
  };
}

void fdrawText(const char *txt,int x, int y, int align, clDeep **font, int start, int max)
{ if(!txt)  return ;
  if(!font) font = gdlfont;
	const char *p = txt ;
	while(*p++ && txt-p<32);
  p-=2; // count string size, max is 32
	while(*p == 0x20) p--;	// remove space at end..
	p++; int sz=p-txt;
    #ifdef use16b
     int sx = (*font)[0];
    #else
     int sx = ((*font)[2])>>16;
    #endif
  if(max) if(sz>max) sz=max ;
  if(align) { x-=sz*sx ; } p=txt ;
  while(x<-sx && sz) { x+=sx ; p++ ; sz-- ; };
  while(sz--){ if(*p>=start && *p<123) drawGfm(font[(*p++) - start],x,y) ; else p++; x+=sx ;
               if(x > (int)WIDTH) return ;
             };
}

void drawText(const char *txt,int x, int y){
	drawText(txt,x,y,0,gdlfont,' ',0);
}

void setGdlfont(clDeep ** font){
	gdlfont = font ;
}

void setGdlfont(clDeep ** font, u32 frmNb){
	unCrunchGfm(font,frmNb);
	gdlfont = font ;
}

clDeep ** getGdlfont(void){
	return gdlfont ;
}

#ifdef WIN32
/*
int mprints(int x,int y,const char * format, ...) // a screen printf
{	if(!gdlfont) return 30 ;
		va_list va; va_start(va,format); wvsprintf(dbg,format,va);
	int valueReturned = 0 ;
	if(x<0 || y<0)
	{	const char *p = dbg ;
		while(*p++ && p-dbg<24); p-=2; // count string size, max is 32
		while(*p == 0x20) p--; p++;	// remove space at end..
		int size = p-dbg; if(!size) return 30 ;
		int fsx,fsy; getGfmSize(&fsx,&fsy,*gdlfont);
		if(x<0) { x = (WIDTH - (size*fsx))>>1; valueReturned=x ; }
		if(y<0) {	y = (HEIGHT - fsy)>>1;
					if(valueReturned) valueReturned&=(y<<16);
					else valueReturned=y ;
				}
	}
		drawText(dbg,x,y,0,gdlfont,' ',24);
		return valueReturned ;
}
*/
//#include <windows.h>
/*void prints(int x,int y,const char * format, ...) // a screen printf
{	if(!gdlfont) return ;
		va_list va; va_start(va,format); wvsprintf(dbg,format,va);
	if(x<0 || y<0)
	{	const char *p = dbg ;
		while(*p++); p--;// count string size
		int size = p-dbg; if(!size) return ;
		int fsx,fsy; getGfmSize(&fsx,&fsy,*gdlfont);
		if(x<0) x = (WIDTH - (size*fsx))>>1;
		if(y<0) y = (HEIGHT - fsy)>>1;
	}
		drawText(dbg,x,y,0,gdlfont,' ',0);
}*/

#endif

void fprints(int x,int y,const char * format, ...) // a screen printf
{	if(!defaultFreetypeFont) return ;
		va_list va; va_start(va,format); vsprintf(dbg,format,va);
		printFreetype(defaultFreetypeFont,(u8*)dbg,x,y+defaultFreetypeFont->height);
}

void prints(int x,int y,const char * format, ...) // a screen printf
{	if(!gdlfont) return ;
		va_list va; va_start(va,format); vsprintf(dbg,format,va);
	if(x<0 || y<0)
	{	const char *p = dbg ;
		while(*p++);
    p--;// count string size
		int size = p-dbg; if(!size) return ;
		int fsx,fsy; getGfmSize(&fsx,&fsy,*gdlfont);
		if(x<0) x = (WIDTH - (size*fsx))>>1;
		if(y<0) y = (HEIGHT - fsy)>>1;
	}
		drawText(dbg,x,y,0,gdlfont,' ',0);
}


/*
void ldrawText(const char *txt,int x, int y, int align, int **font,int max=0)
{ const char *p = txt ; while(*p++); p-=2; while(*p == 0x20) p--; p++; int sz=p-txt, sx=((*font)[2])>>16, way=1 ;
  if(max) if(sz>max) sz=max ;
  if(align) { x-=sz*sx ; way=-1 ; } p=txt ;
  int c=0 ; while(x<-sx && sz) { x+=sx ; p++ ; sz-- ; };
  while(sz--){  int lt = *p ; if(!lt) return ;
				if(lt >= 'A' && lt <= 'Z') { lt-='A'; lt+='a'; } // down to a-z if need
				if(lt != ' ' && ((lt >= 'a' && lt <= 'z')||(lt >= '0' && lt <= '9')||lt=='\''||lt=='.'||lt=='-') )
					{			if(lt>='a')		drawGfm(font[lt-'a'],x,y);
						else	if(lt == '\'')	drawGfm(font[26+10],x,y);
						else	if(lt == '.')	drawGfm(font[26+11],x,y);
						else	if(lt == '-')	drawGfm(font[26+12],x,y);
						else					drawGfm(font[lt-'0'+26],x,y);
					}
				x+=sx ; p++;
                if(x > 320) return ;
             };
}*/

void drawInt(int n, int x, int y, const char *method, int align, clDeep **font, int start){
  char t[16];
  sprintf(t,method,n);
  drawText(t,x,y,align,font,start);
}


#endif
