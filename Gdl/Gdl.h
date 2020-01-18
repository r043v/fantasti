/************
              Gdl²
                    ***********/


/* under creative commons by-nc-sa 4.0 */

/* Gdl complete source and tools are © 2002/2020 noferi mickaël - noferov@gmail.com - r043v/dph */

/******** changelog.

  .*/

#ifndef _Gdl_
#define _Gdl_

#include "./config.h"

extern clDeep *pixel; // pointer to framebuffer used by blit routines
extern clDeep *scrStart,*scrEnd;

extern u32 isKeyPress;

u32  Gdl_init(const char* appTitle, u32 width, u32 height);	// open framebuffer
void Gdl_flip(void);					// flip it
void Gdl_exit(u32 exitProcess=1);		// and close it

void Gdl_Sleep(u32 timeInMs);

void Gdl_updateMsg(void); // refresh event.. (key mouse ect..)

// set callback
void Gdl_iniCallback( void	(*onKey)	    (int key, int pushed),
					  void	(*onClick)		(int button, int clicked),
					  void	(*onExit)       (void),
						void	(*onUpdate)	(void),
					  void	(*onMove)		(void),
					  void	(*onWhell)		(int way),
					  void	(*onFocus)		(int focus),
 					  void	(*onFileDrop)	(const char*path),
					  void  (*onAfterZoom)	(void*bf,u32 w,u32 h,u32 pitch)
      );

//#include "./tablesMath.h"

extern u32 tick ; // current time will be copied here after eatch screen flip
extern char dbg[2048] ; // debugString
extern u32 mousex, mousey;
extern u32 mouseLeftState;
extern u32 mouseMidlState;
extern u32 mouseRightState;
extern u32 isAppActive;
extern u32 isMouseHere;

extern struct freetypeFont * defaultFreetypeFont;

void printl(const char * format, ...); // print to log, replace printf
void printlr(const char * format, ...); // print to log with an auto line return
void log(const char * info=0,const char * path=0, int lineReturn=0);
void mem2log(void * src, u32 size, const char * path=0);

void Gdl_setAppPower(u32 value);

/* Animate routines */

void clrScr(clDeep color=0);


struct	anim
{	u32	lastTime ;
	u32	frmTime  ;
	u32	curentFrm;
	u32	frmNumber;
	u32	animType;
	void (*onfinish)(struct anim **);
	void (*onflip)(struct anim **);
	void (*onplay)(struct anim **);
	clDeep	**Gfm, **iGfm;
};

int Animate(struct anim **b) ;
void playAnim(struct anim **b, int x, int y, u32 way=0);
void playAnim(struct anim **b, clDeep *screen);
void drawAnim(struct anim **b, int x, int y, u32 way=0);
void drawFrm(struct anim **b, int x, int y, int frm, u32 way=0);
void  ifreeAnim(struct anim *a);
void mifreeAnim(struct anim **a, u32 nb);
clDeep** flipGfm(clDeep **Gfm, u32 nb);
struct anim * setAnim( clDeep **Gfm, u32 nb,
                       u32 frmTime, u32 animType=0,
                       void (*onfinish)(struct anim**)=0,
                       void (*onflip)(struct anim**)=0,
                       void (*onplay)(struct anim**)=0
                     );
void resetAnim(struct anim **a);

/* free list - manage list of allocated adress */

void addFreeEntry(void *add, void**list=0);
void ** createFreeList(u32 maxEntry=512);
void freeAllEntry(void **list=0);
void * allocAndAddFree(u32 allocSize,void **list=0);


/* line blitting */
void  ligne(int xi,int yi,int xf,int yf, clDeep *line);
void _ligne(int xi,int yi,int xf,int yf, clDeep *line);
void aline(int x, int y, int angle, int size, clDeep *line);
u32 getRotatePos(int x, int y, int angle, int size);
void savelineMove(int x,int y,int xx,int yx,int*mx,int*my);

/* text output */
void drawText(const char *txt,int x, int y, int align, clDeep **font, int start=' ', int max=0);
void drawText(const char *txt,int x, int y);
void drawInt(int n, int x, int y, const char *method, int align, clDeep **font, int start=' ');
void prints(int x,int y,const char * format, ...); // a screen printf
int mprints(int x,int y,const char * format, ...); // a screen printf

/* font set & save */
void setGdlfont(clDeep ** font);
void setGdlfont(clDeep ** font, u32 frmNb);
clDeep ** getGdlfont(void);

#ifdef useFreetype
  #include <ft2build.h>
  #include FT_FREETYPE_H
  #include "unicode/utf8.h"

	struct glyph {
		clDeep * gfm;
		int left;
		int top;
	};

  struct freetypeFont {
    FT_Face face;
		FT_Face fallback;
    u32 height;
    //clDeep ** font;
		struct glyph ** font;
    FT_Bitmap * bitmap;
    clDeep * buffer;
		u32 r,g,b,minGrayValue;
  };

	void fprints(int x,int y,const char * format, ...); // a screen printf

	void initFreetype( void );
	void loadFreetypeGlyph( struct freetypeFont * f, UChar32 c );
	void loadFreetypeChar( struct freetypeFont * f, UChar32 c );
	void loadFreetypeChars( struct freetypeFont * f, char * s );
	void loadFreetypeChars( struct freetypeFont * f, u8 * s );
	struct freetypeFont * loadFreetypeFont( u8 * font, u32 fontSize, u32 height );
	struct freetypeFont * loadFreetypeFont( const char * font, u32 height );
	void setFreetypeFont( struct freetypeFont * f );
	void setFallbackFont( struct freetypeFont * f, const char * font );
	void releaseFreetypeFont( struct freetypeFont * f );
	void releaseFreetype( void );
	void printFreetype( struct freetypeFont * f, char * s, int x, int y );
	void printFreetype( struct freetypeFont * f, u8 * s, int x, int y );
	void setFreetypeColor( struct freetypeFont * f, u8 r, u8 g, u8 b, u8 minGrayValue );
#endif

/* other.. */
u32 getSize(FILE *f);	// return size of file gived in argument
unsigned char * loadFile(const char *path, u32*size=0);	// load a file from disk
u32 writeFile(const char *path, void*buffer, u32 size);// the same, but for write a file
u32 fileExist(const char*path); // is specified file exist ?
void bin2h(const char *file, const char * name);	// convert a file into an header (.h)
u32 countFps(void);	/* count number of frame/second, call this routine only one time per frame */

u32 divide(u32 divadeWhat, u32 byHowMany);

void showCursor(u32 x, u32 y,u32 x1, u32 y1, u32 size, u32 size1, clDeep color, clDeep color1,u32 time);

u32 computeDec(u32 value);
void logAsciiTable(void);
u32 isAPath(char*path);

extern u8*keyArray;
extern u8*lastKey;
u32 keyUp(u32 key);
u32 keyRelease(u32 key);
u32 keyPush(u32 key);
void clearLastKey( void );

int mLoadTGA(char *file, u8 *outpix, u32 *pal, u32 *clNb, u32 *w, u32 *h, int size);
/*
u32 Gdl_playSong(const char * songPath);
u32 Gdl_playSong(u8* song,u32 size);
u32 Gdl_playSong(u8*rar,u32 rarSize,const char * songPath); // memory rar dedicated playSong
void Gdl_fadeAndStopSong(u32 fadeSpeed=80);
u32 Gdl_setSfx(u8* sfxSong,u32 size);
void Gdl_playSfx(u32 splNb, u32 freq, u32 volume=63, u32 chn=255);
*/

#include "./misc/rate.h"

#include "./output/graphic/gfm/Gfm.h"	/* include the bliting routine header */
//#include "./input/xml/xml.h"  /* include xml support header */
#include "./output/graphic/map/map.h"  /* include map blitting header */
//#include "./output/sound/mymixr/myMixr.h"		// sound mixer
//#include "./input/mod/mdpl.h"			// mod support
//#include "./output/sound/minifmod/minifmod.h"	// xm player
//#include "./input/rar/unrarlib.h"	// rar support

//#include "../main/entity.h"

class console
{	public:
	u32  x,y,px,py,sx,sy,sxpx,sypx,scroll;
	u32  showCursor;

	clDeep  textcolor;
	clDeep  bgColor;

	char *buffer;
	char *currentChar;
	char *currentLine;
	char *temp;

	u32  bufferFree;
	u32  bufferSize;
	u32  bufferUsed;
	u32  linefill;
	u32  newline;
	void set(int px,int py,u32 sx,u32 sy);
	//console();
	void kill(void);

	void print(const char * format, ...);
	void waitPrint(const char * format, ...);
	void printxy(u32 x,u32 y,const char * format, ...);
	void waitPrintxy(u32 x,u32 y,const char * format, ...);

	void jumpLine(void);

	void clear(void);
	void  draw(void);

	u32 save(const char * path);
	u32 load(const char * path);
};

extern console shell;
extern u32 drawGdlShell;

//#include "./output/sound/minifmod/minifmod.h"	// xm player

#ifdef __arm__
extern "C" { void asm_memcpy(void*,void*,unsigned int); }
#endif

#endif
