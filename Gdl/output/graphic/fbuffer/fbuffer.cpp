
#ifndef _gdlfbuffer_
#define _gdlfbuffer_

#include "../../../Gdl.h"

	u32 WIDTH, HEIGHT;

    u32 tick=0;		// curent tick will be copied here
    extern clDeep *pixel;

    #ifdef useSdl
        SDL_Surface *sdlScreen = NULL;
        SDL_Joystick *joy = NULL;
    #endif

u32 done;

void	(*_onKey)				(int key, int pushed)=0;
void	(*_onClick)			(int button, int clicked)=0;
void	(*_onMove)			(void)=0;
void	(*_onExit)			(void)=0;
void	(*_onUpdate)		(void)=0;
void	(*_onWhell)			(int way)=0;
void	(*_onFocus)			(int focus)=0;
void	(*_onFileDrop)	(const char*path)=0;

u32 Gdl_processMsg(u32 msg, u32 arg1, u32 arg2);

u32 mouseLeftState=0;
u32 mouseMidlState=0;
u32 mouseRightState=0;
u32 mousex, mousey ;
u32 isAppActive = 1;
u32 isMouseHere = 0;

extern u32 isKeyPress;

void Gdl_updateMsg(void) // will manage event and launch callback
{		if( _onUpdate ) _onUpdate();
    #ifdef useSdl
    SDL_Event event;
	/* Check for events */
	while (SDL_PollEvent (&event))
	{     u32 key;
		switch (event.type){
			case SDL_KEYDOWN:
				key = event.key.keysym.sym;
//				shell.print("key %i\n",key);
				if(key < keyArraySize)
				{	if(_onKey) _onKey(key,1);
					lastKey[key]=keyArray[key];
					keyArray[key]=1;
					isKeyPress += key;
					//shell.print("* %i\n",key);
  			}
			break;
			case SDL_KEYUP:
        key = event.key.keysym.sym;
        if(key < keyArraySize)
        {	if(_onKey) _onKey(key,0);
        	lastKey[key]=keyArray[key];
          keyArray[key]=0;
					if( isKeyPress > key ) isKeyPress -= key; else isKeyPress = 0;
        }
			break;
			case SDL_JOYBUTTONDOWN:
				/* if press Start button, terminate program */
				if ( event.jbutton.button == 8 )
					done = 1;
				break;
			case SDL_JOYBUTTONUP:
				break;
		    case SDL_MOUSEMOTION:
                mousex = event.motion.x;
                mousey = event.motion.y;
                isMouseHere = 1;
                if(_onMove) _onMove();
                break;
            case SDL_MOUSEBUTTONDOWN:
                isMouseHere = 1;
                switch(event.button.button)
                {  case SDL_BUTTON_LEFT:   mouseLeftState=1;  if(_onClick)_onClick(-1,1); break;
                   case SDL_BUTTON_MIDDLE: mouseMidlState=1;  if(_onClick)_onClick( 0,1); break;
                   case SDL_BUTTON_RIGHT : mouseRightState=1; if(_onClick)_onClick( 1,1); break;
                }; break;
            case SDL_MOUSEBUTTONUP:
                isMouseHere = 1;
                switch(event.button.button)
                {  case SDL_BUTTON_LEFT:   mouseLeftState=0;  if(_onClick)_onClick(-1,0); break;
                   case SDL_BUTTON_MIDDLE: mouseMidlState=0;  if(_onClick)_onClick( 0,0); break;
                   case SDL_BUTTON_RIGHT : mouseRightState=0; if(_onClick)_onClick( 1,0); break;
                }; break;
            case SDL_ACTIVEEVENT:
								isKeyPress = 0; // reset key count on focus in/out
								if(event.active.state != SDL_APPMOUSEFOCUS)
                	isAppActive = event.active.gain;
                else
									isMouseHere = event.active.gain;
            break;
			case SDL_QUIT:
				//done = 1;
				exit(1);
				break;
			default:
				break;
		}
	}
    #endif
}

 /* Gdl callback definition */

void Gdl_iniCallback( void	(*onKey)	    (int key, int pushed)=0,
					  void	(*onClick)			(int button, int clicked)=0,
					  void	(*onMove)				(void)=0,
					  void	(*onExit)				(void)=0,
						void	(*onUpdate)			(void)=0,
					  void	(*onWhell)			(int way)=0,
					  void	(*onFocus)			(int focus)=0,
 					  void	(*onFileDrop)		(const char*path)=0,
					  void  (*onAfterZoom)	(void*bf,u32 w,u32 h,u32 pitch)=0
      )
  { if(onKey)      _onKey       = onKey;
    if(onClick)    _onClick     = onClick;
    if(onFocus)    _onFocus     = onFocus;
    if(onMove)     _onMove      = onMove;
    if(onFileDrop) _onFileDrop  = onFileDrop;
    if(onExit)     _onExit      = onExit;
		if(onUpdate)   _onUpdate    = onUpdate;
    if(onWhell)    _onWhell     = onWhell;
	//if(onAfterZoom) ptc_setAfterZoomCallBack(onAfterZoom);
  }

void inikey(void);

u32 Gdl_init(const char* appTitle, u32 width, u32 height) // open you a framebuffer
{   HEIGHT = height; WIDTH = width;
    inikey();
    #ifdef useSdl
    	if(SDL_Init (SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0)
        {  fprintf (stderr, "sdl init error (%s)\n",SDL_GetError()); return 0;
    	}

    	sdlScreen = SDL_SetVideoMode (WIDTH, HEIGHT, 32, SDL_HWSURFACE | SDL_DOUBLEBUF );
			SDL_SetColorKey(sdlScreen,false,0);

/*SDL_Window *sdlScreen = SDL_CreateWindow(appTitle,
                          SDL_WINDOWPOS_UNDEFINED,
                          SDL_WINDOWPOS_UNDEFINED,
                          320, 240,
                          SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL
												);
*/
			//printf("%u 0x%8x",sdlScreen->format->BitsPerPixel,sdlScreen->format->format);

    	if(!sdlScreen)
        {	printf ("error on set %ix%i.%i video mode (%s)\n",
                      WIDTH,HEIGHT,deepByte,SDL_GetError()); return 0;
    	}
        //SDL_WM_SetCaption(appTitle,0);
	   //SDL_ShowCursor(SDL_DISABLE);

    	if(SDL_NumJoysticks() > 0)
        {	joy = SDL_JoystickOpen(0);
    		if(!joy)
            {	fprintf (stderr, "sdl link to joystick error (%s)\n",SDL_GetError()); return 0;
    		}
    	}

    	pixel = (clDeep*)(sdlScreen->pixels);
    	SDL_UnlockSurface(sdlScreen);
   	#endif

   	#ifdef useRlyeh
   	       gp2x_init(1000, deepByte, 11025,16,1,60);
           pixel = gp2x_screen15;
   	#endif

   	#ifdef startAppWithFullPower
   	   Gdl_setAppPower(1);
     #else
       Gdl_setAppPower(0);
	#endif
   	//atexit (Gdl_exit);
		setOutBuffer(pixel,width,height);
   	fullBlitLimit();
    return 1;
}

u32 letMeUseSomePower = 0;
void Gdl_setAppPower(u32 value)
{	letMeUseSomePower = (value>0);
}

console shell;
u32 drawGdlShell = false;

void Gdl_flip(void) // flip the framebuffer
{	if(!pixel) return;
	//printf("%s\n","flip" );

	if(drawGdlShell) shell.draw();

    #ifdef useSdl
      SDL_LockSurface(sdlScreen);
      //SDL_UpdateRect(screen, 0, 0, 0, 0);
  	  SDL_Flip(sdlScreen);
      SDL_UnlockSurface(sdlScreen);
      Gdl_updateMsg();
      if(letMeUseSomePower){
             #ifdef sleepWhenNoFocus
                    SDL_Delay(10*(!isAppActive));
             #else
                    SDL_Delay(0);
             #endif
      } else SDL_Delay(10);
      tick = GetTickCount();
			//printf("%u\n",tick);
    #endif

   	#ifdef useRlyeh
   	  gp2x_video_flip();
      pixel = gp2x_screen15;
      resetScreenSize();
      tick = GetTickCount();
   	#endif
}

/*void myAtExit(void)
{	Gdl_exit(1);
}*/

void Gdl_Sleep(u32 timeInMs)
{
	#ifdef useSdl
      SDL_Delay(timeInMs);
   	#endif

    #ifdef useRlyeh

    #endif

}

void Gdl_exit(u32 exitProcess) // close the framebuffer
{//   log();
//	log("close gdl..\n");
	if(keyArray) { free(keyArray); keyArray=0; }
  //  log("key array cleaned\n");

    #ifdef useRlyeh
			gp2x_deinit();
    #endif

//	log("framebuffer closed\n");

	if(exitProcess)
	{//	log("execute on exit event\n");
		if(_onExit) _onExit();
		 //else Gdl_fadeAndStopSong();
	//	log("on exit event launch\n");
		#ifdef GP2X
			chdir("/usr/gp2x"); //go to menu
			execl("gp2xmenu","gp2xmenu",NULL);
		#endif
	//	log("close process\n");
		#ifdef WIN32
			ExitProcess(0);
		#endif

		#ifdef useSdl
			//printf("exit");
				SDL_Quit();
		#endif
	}
}

#endif
