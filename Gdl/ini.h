#ifndef _gdlInit_
#define _gdlInit_

/*** include & define standart thing ***/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#define  u8 unsigned  char
#define  s8  signed   char
#define u16 uint16_t
#define s16  int16_t
#define u32 uint32_t
#define s32  int32_t

extern u32 WIDTH, HEIGHT;

/*** init gdl with selected options.. ***/

#ifdef LINUX
  #define useSdl
  #define use32b
#endif

#ifdef WIN32
       #ifndef useSdl
        #define usePtc
       #endif
#endif

#ifdef GP2X
       #ifndef useSdl
        #define useRlyeh
       #endif
#endif

#ifdef  useSdl
  #include <SDL/SDL.h>

  #ifdef use32bIfPossible
    #define use32b
  #else
    #define use16b
  #endif

  #define GetTickCount SDL_GetTicks
  #define keyArraySize 512
#endif

#ifdef useRlyeh
            #include "./output/graphic/fbuffer/rlyeh/minimal.h"
            #define use16b
            #define GetTickCount gp2x_timer_read
#endif

#ifdef  use16b
       #define clDeep     u16
       #define clDeepDec  1
       #define clDeepSze  2
       #define deepByte    16
#else
       #define clDeep     u32
       #define clDeepDec  2
       #define clDeepSze  4
       #define deepByte    32
#endif

/*switch( event.key.keysym.sym ){
  case SDLK_j: // A
  case SDLK_h: // fn a
  case SDLK_k: // B
  //case SDLK_l: // fn B
  case SDLK_u: // X
  case SDLK_i: // Y
  case SDLK_y: // fn X
  case SDLK_o: // fn Y
//    case SDLK_PLUS: // fn start 270
  case 270:
  case SDLK_RETURN: // start
  case SDLK_SPACE: // select
  case SDLK_MINUS: // fn select 269
  case 269:
  case SDLK_UP:
  case SDLK_DOWN:
  case SDLK_LEFT:
  case SDLK_RIGHT:
  case SDLK_BACKSPACE: // fn menu
  case SDLK_h: // fn A
  case SDLK_h: // fn START
  case SDLK_l: // fn B
  case SDLK_ESCAPE: // menu
*/

#ifdef useSdl
  #ifdef GAMESHELL
    #define ka 106 // j
    #define kb 107 // k
    #define kx 117 // u
    #define ky 105 // i
    #define kfna 104 // h
    #define kfnb 108 // l
    #define kfnx 121 // y
    #define kfny 111 // o
    #define kselect 32 // space
    #define kfnselect 269 // -
    #define kmenu 27 // escape
    #define kfnmenu 8 // backspace
//    #define kstart 13  // return
    #define kfnstart 270 // +
    #define klk1 278 // home
    #define klk2 280 // pageup
    #define klk4 281 // pagedown
    #define klk5 279 // end
    #define kfnlk1 kfna
    #define kfnlk2 kfnx
    #define kfnlk4 kfny
    #define kfnlk5 kfnb
/*    #define kleft 276 // left
    #define kup 273 // up
    #define kright 275 // right
    #define kdown 274 // down
*/
    #define kspace SDLK_SPACE
    #define kenter SDLK_RETURN
    #define kstart SDLK_RETURN
    #define kctrl  0
    #define kfin   SDLK_END
    #define kshift 0
    #define kleft  SDLK_LEFT
    #define kup    SDLK_UP
    #define kright SDLK_RIGHT
    #define kdown  SDLK_DOWN
    #define kesc   SDLK_ESCAPE
  #else
    #define kspace SDLK_SPACE
    #define kenter SDLK_RETURN
    #define kstart SDLK_RETURN
    #define kctrl  0
    #define kfin   SDLK_END
    #define kshift 0
    #define kleft  SDLK_LEFT
    #define kup    SDLK_UP
    #define kright SDLK_RIGHT
    #define kdown  SDLK_DOWN
    #define kesc   SDLK_ESCAPE
  #endif
#endif

#define killKeyFront() keyUp(0xffff)

#define CHAR_PADDING 1 /* padding between chars */
#define CHAR_SPACE 4 /* space char width */

/*#ifdef useFreetype
  #include <ft2build.h>
  #include FT_FREETYPE_H
  #include "unicode/utf8.h"
#endif*/

#endif
