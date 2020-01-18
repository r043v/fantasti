// transparent blit routine for Gdl²
// (C) 2k4/2k6 by r043v, cannot be used for any commercial purpose

#include "../../../Gdl.h"

extern u32 tick;

clDeep * data2Gfm(unsigned char *data);
void unCrunchGfm(clDeep ** gfm, u32 frmNb);
void drawGfm(clDeep *Gfm, int x, int y);
void udrawGfm(clDeep *Gfm,clDeep *scr);

// blit zone definition routines ..

// delimate current blit zone
void   setBlitLimit(u32 x, u32 y, u32 x1, u32 x2);
void setBlitLimit(clDeep *start, clDeep *end, u32 widthStart, u32 widthEnd);
void setBlitLimit(u32 widthStart, u32 widthEnd);

// save or load a blit zone
void  saveBlitLimit(void); // save current blit limit
void  loadBlitLimit(void); // retreve saved blit limit

void internalLoadBlitLimit(void);
void internalSaveBlitLimit(void);

void setOutBuffer(clDeep*bf,int w, int h);

// maximize blit zone
void fullBlitLimit(void); // define blit limit as full screen

	#define xy2scr(x,y)		(&pixel[(x)+(y)*WIDTH])
	//#define getGfmSize(x,y,gfm)	if(x)*(x)=(((gfm)[2])>>16);if(y)*(y)=(((gfm)[2])&0xffff)

//#include "../../../Gdl.h"

clDeep *scrStart,*scrEnd,*pixel ;
int frmWidth, frmWidthStart, frmWidthEnd, bufWidth, bufHeight;

clDeep *saved_scrStart=0, *saved_scrEnd=0, *saved_screen=0 ;
u32 saved_frmWidth=0, saved_frmWidthStart=0, saved_frmWidthEnd=0;

clDeep *isaved_scrStart=0, *isaved_scrEnd=0, *isaved_screen=0 ;
u32 isaved_frmWidth=0, isaved_frmWidthStart=0, isaved_frmWidthEnd=0;


void setOutBuffer(clDeep*bf,int w, int h)
{	pixel = bf;
	bufWidth = w;
	bufHeight = h;
	fullBlitLimit();
}

void fullBlitLimit(void)
{	scrStart  = pixel;
	scrEnd    = &pixel[bufWidth*bufHeight-1];
	frmWidth  = bufWidth;
	frmWidthStart = 0;
	frmWidthEnd   = bufWidth;
}

void saveBlitLimit(void)
{	saved_scrStart  = scrStart;
	saved_scrEnd    = scrEnd;
	saved_frmWidth  = frmWidth;
	saved_frmWidthStart = frmWidthStart;
	saved_frmWidthEnd   = frmWidthEnd;
}

void loadBlitLimit(void)
{	scrStart  = saved_scrStart;
	scrEnd    = saved_scrEnd;
	frmWidth  = saved_frmWidth;
	frmWidthStart = saved_frmWidthStart;
	frmWidthEnd   = saved_frmWidthEnd;
}

void internalSaveBlitLimit(void)
{	isaved_scrStart  = scrStart;
	isaved_scrEnd    = scrEnd;
	isaved_frmWidth  = frmWidth;
	isaved_frmWidthStart = frmWidthStart;
	isaved_frmWidthEnd   = frmWidthEnd;
}

void internalLoadBlitLimit(void)
{	scrStart  = isaved_scrStart;
	scrEnd    = isaved_scrEnd;
	frmWidth  = isaved_frmWidth;
	frmWidthStart = isaved_frmWidthStart;
	frmWidthEnd   = isaved_frmWidthEnd;
}

void setBlitLimit(clDeep *start, clDeep *end, u32 widthStart, u32 widthEnd)
{	scrStart  = start;
	scrEnd    = end  ;
	frmWidth  = widthEnd-widthStart;
	frmWidthStart = widthStart;
	frmWidthEnd   = widthEnd;
}

void setBlitLimit(u32 widthStart, u32 widthEnd)
{	u32 width = widthEnd - widthStart;
	scrStart  = &pixel[widthStart];
	scrEnd    = &pixel[bufWidth*(bufHeight-1)+widthEnd];
	frmWidth  = width;
	frmWidthStart = widthStart;
	frmWidthEnd   = widthEnd;
}

void setBlitLimit(u32 x, u32 y, u32 x1, u32 y1)
{	u32 width = x1 - x;
	scrStart  = &pixel[x+y*bufWidth];
	scrEnd    = &pixel[x1+y1*bufWidth];
	frmWidth  = width;
	frmWidthStart = x;
	frmWidthEnd   = x1;
}


void getGfmRealSize(clDeep *Gfm, int *up, int *down)
{   int sy = Gfm[2]&0xffff ;
    int cnt=0, c ; clDeep *gPtr = Gfm + 3 ; int last=0 ;
    *up=*down=-1 ;

    //printf("\n* getGfmRealSize .. sy %i",sy) ;

  while(cnt<sy)
  {    c = *gPtr++ ; if(*down == -1){ if(c) *down = cnt ; }
                     else if(!c && last) *up = cnt ;
                     last = c ;

       while(c--) { gPtr++ ; gPtr += *gPtr++ ; };
       cnt++ ;
  };
  if(*down == -1) *down=0 ;
  if(*up == -1) *up = sy-1 ;
  //printf("up %i .. down %i\n",*up,*down) ;
}

clDeep * Gfm2array(clDeep *Gfm){
/*
	u32 z = 16;
	clDeep * zz = Gfm;
	while(z--)
		printf("%08x ",*zz++);

	printf("\n");
*/
	Gfm += 2;
	u32 sx = (*Gfm)>>16, sy = (*Gfm++)&65535, px = sx*sy, b=px*sizeof(clDeep) ;
//	printf("%ux%u = %u = %uB\n",sx,sy,px,b);
  clDeep *array = (clDeep *)malloc(b) ;
  memset(array,0xff,b) ;
	clDeep * o = array ;

/*
	while(c < sy)
		 {  lnb = *Gfm++ ;
				while(lnb--) { scr += *Gfm++ ; sze = *Gfm++ ;

											 while(sze>1)
												 { *scr++ = *Gfm++;
													 *scr++ = *Gfm++;
													 sze-=2;
												 };
											 if(sze) *scr++ = *Gfm++;
				};  scr = screen + bufWidth*(++c) ;
		 };
*/

	u32 y = 0, lines, size ;
  while(y < sy){
//		printf("* line %u/%u\n",y,sy-1);
		lines = *Gfm++ ;
//		printf("%u lines\n",lines);
		while(lines--){
//			printf("jump %u\n",*Gfm);
			array += *Gfm++ ; // jump
			size = *Gfm++ ; // px length
//			printf("copy %u\n",size);
			memcpy( array, Gfm, size*sizeof(clDeep) ) ;
			Gfm += size ; array += size ;
		};
		array = o + sx*(++y) ;
  };

/*	for( y=0;y<sy;y++ ){
		clDeep * a = &o[y*sx];
		for( u32 x=0;x<sx;x++ )
			printf("%u",*a++);
		printf("\n");
	}
*/
	return o ;
}

void Gfm2cldArray(clDeep *Gfm, u8 * out)
{ int sx = Gfm[2]>>16, sy = Gfm[2]&0xffff ;
	u8 * o = out;
  //clDeep *array=(clDeep *)malloc(sx*sy*4) ; clDeep *o=array ;
  memset(out,0xFF,sx*sy) ;
  int cnt=0, c ; clDeep *gPtr = Gfm + 3 ;
  int size ;
  while(cnt<sy)
  {    c = *gPtr++ ;
       while(c--) { o += *gPtr++ ; size = *gPtr++ ;
                    memset(o,0,size) ;
                    gPtr+=size ; o+=size ;
                  }; o = out + sx*(++cnt) ;
  };
}

void drawGfmFakeHeight(clDeep *Gfm, int x, int y, u32 height, int way){
	#ifdef use32b
	 u32 sy = Gfm[2]&65535 ;
	#else
	 u32 sy = Gfm[1];
	#endif

	y += (sy - height)*way;

	drawGfm( Gfm, x, y );
}

void drawGfm(clDeep *Gfm, int x, int y)
{
     #ifdef use32b
      u32 sx = Gfm[2]>>16 ; u32 sy = Gfm[2]&65535 ;
     #else
      u32 sx = Gfm[0]; u32 sy = Gfm[1];
     #endif

     if(x >= frmWidthStart && x + (int)sx < frmWidthEnd)
		{		int p = y*WIDTH+x;
				//printf("%ix%i >>> %i\n",x,y,p);
				udrawGfm(Gfm,&pixel[ p ]) ;
                return;
        }

     if(x < (int)(frmWidthStart-sx) || x > frmWidthEnd) return ;// out of screen on x

     if(x >= frmWidthStart) // clip right only
     {   u32 max = frmWidthEnd-x ;  u32 lnb, sze ;
			 	int p = y*WIDTH+x;
         clDeep *scr = &pixel[p] ;
         if(scr > scrEnd) return ; // out of screen at down

         u32 upClip = (scr + sy*WIDTH > scrEnd) ; // is clipped at down ?

         #ifdef use32b
          Gfm += 3;
         #else
          Gfm += 2;
         #endif

         if(scr < scrStart) // is clipped at up ?
         {	if(scr + sy*WIDTH < scrStart) return ; // out of screen at up
            do{ lnb = *Gfm++ ;
                while(lnb--) { Gfm++ ; Gfm += *Gfm++ ; };
                scr += WIDTH ; sy-- ;
            } while(scr < scrStart) ;
         }

         clDeep *screen = scr ; u32 c = 0 ; clDeep * lend ;
         while(c < sy)
         {  lnb = *Gfm++ ; lend = scr + max ;
            while(lnb--) { scr += *Gfm++ ; sze = *Gfm++ ;
                           if(scr + sze < lend) memcpy(scr,Gfm,sze<<clDeepDec) ;
                           else if(scr < lend)  memcpy(scr,Gfm,(lend-scr)<<clDeepDec) ;
                           Gfm += sze ;    scr+=sze ;
            };  scr = screen + WIDTH*(++c) ;
            if(upClip) if(scr > scrEnd) return ;
         };
     } else if(x+(int)sx < frmWidthEnd) { // clip left only
			 int p = y*WIDTH + frmWidthStart;
         u32 lnb, sze ; clDeep *s = &pixel[p] ;

         if(s > scrEnd) return ; // out of screen at up
         u32 upClip = (s + sy*WIDTH > scrEnd) ; // is clipped at up ?

         #ifdef use32b
          Gfm += 3;
         #else
          Gfm += 2;
         #endif

         if(s < scrStart) // is clipped at down ?
         {  if(s + sy*WIDTH < scrStart) return ; // out of screen at down
            do{ lnb = *Gfm++ ;
                while(lnb--) { Gfm++ ; Gfm += *Gfm++ ; };
                s += WIDTH ; sy-- ;
            } while(s < scrStart) ;
         }

         clDeep*scr = s+x ;
         scr -= frmWidthStart ;
         clDeep *screen = scr ;
         u32 c=0 ; u32 size ;

         while(c < sy)
         {  lnb = *Gfm++ ;
            while(lnb--) { scr += *Gfm++ ; sze = *Gfm++ ;
                           if(scr >= s)   memcpy(scr,Gfm,sze<<clDeepDec) ;
                           else if(scr + sze > s) { size = (scr + sze)-s ;
                                                    memcpy(s,Gfm+(sze-size),size<<clDeepDec) ;
                                                  }
                           Gfm += sze ;    scr+=sze ;
            };  scr = screen + WIDTH*(++c) ; s = scr+frmWidthStart ; s -= x ;
            if(upClip) if(s > scrEnd) return ;
         };
     } else { // clip left and right
			 int p = y*WIDTH + frmWidthStart;
		 			u32 lnb, sze ; clDeep *s = &pixel[p] ;
		 			u32 max = frmWidthEnd-x ;
         if(s > scrEnd) return ; // out of screen at up
         u32 upClip = (s + sy*WIDTH > scrEnd) ; // is clipped at up ?

         #ifdef use32b
          Gfm += 3;
         #else
          Gfm += 2;
         #endif

         if(s < scrStart) // is clipped at down ?
         {  if(s + sy*WIDTH < scrStart) return ; // out of screen at down
            do{ lnb = *Gfm++ ;
                while(lnb--) { Gfm++ ; Gfm += *Gfm++ ; };
                s += WIDTH ; sy-- ;
            } while(s < scrStart) ;
         }

         clDeep*scr = s+x ;
         scr -= frmWidthStart ;
         clDeep *screen = scr ;
         u32 c=0 ; u32 size ; clDeep * lend ;

         while(c < sy)
         {  lnb = *Gfm++ ; lend = scr + max ;
            while(lnb--) { scr += *Gfm++ ; sze = *Gfm++ ;



			// left
						if(scr >= s)
						{	//memcpy(scr,Gfm,sze<<clDeepDec) ;


							if(scr + sze < lend) memcpy(scr,Gfm,sze<<clDeepDec);
								else
							if(scr < lend)	memcpy(scr,Gfm,(lend-scr)<<clDeepDec);




						}
							else
						if(scr + sze > s) {	size = (scr + sze)-s;



											if(scr + sze < lend)
												memcpy(s,Gfm+(sze-size),size<<clDeepDec);
											//memcpy(scr,Gfm,sze<<clDeepDec);
												else
											if(scr < lend)
											memcpy(s,Gfm+((lend-scr)-size),size<<clDeepDec);
												//memcpy(scr,Gfm,(lend-scr)<<clDeepDec);



                                          }

						Gfm += sze ;    scr+=sze ;
            };


			scr = screen + WIDTH*(++c) ; s = scr+frmWidthStart ; s -= x ;
            if(upClip) if(s > scrEnd) return ;
         };

	}
}

/*
void drawGfm(clDeep *Gfm, int x, int y)
{
      int sx = Gfm[2]>>16 ; u32 sy = Gfm[2]&65535 ;

//printf("\n%ux%u",sx,sy);

     if(x >= frmWidthStart && x + sx < frmWidthEnd)
		{		udrawGfm(Gfm,&pixel[y*bufWidth+x]) ;
                //printf("%u.%u\n",x,y);
                return;
        }

//printf("%u.%u %u.%u %u\n",x,y,frmWidthStart,frmWidthEnd,sx);

     if(x < (frmWidthStart-sx) || x > frmWidthEnd) return ;// out of screen on x




     if(x >= frmWidthStart) // clip right only
     {   u32 max = frmWidthEnd-x ;  u32 lnb, sze ;
         clDeep *scr = &pixel[y*bufWidth+x] ;
         if(scr > scrEnd) return ; // out of screen at down

         u32 upClip = (scr + sy*bufWidth > scrEnd) ; // is clipped at down ?

          Gfm += 3;

         if(scr < scrStart) // is clipped at up ?
         {  if(scr + sy*bufWidth < scrStart) return ; // out of screen at up
            do{ lnb = *Gfm++ ;
                while(lnb--) { Gfm++ ; Gfm += *Gfm++ ; };
                scr += bufWidth ; sy-- ;
            } while(scr < scrStart) ;
         }

         clDeep *screen = scr ; u32 c = 0 ; clDeep * lend ;
         while(c < sy)
         {  lnb = *Gfm++ ; lend = scr + max ;
            while(lnb--) { scr += *Gfm++ ; sze = *Gfm++ ;
                           if(scr + sze < lend) memcpy(scr,Gfm,sze<<clDeepDec) ;
                           else if(scr < lend)  memcpy(scr,Gfm,(lend-scr)<<clDeepDec) ;
                           Gfm += sze ;    scr+=sze ;
            };  scr = screen + bufWidth*(++c) ;
            if(upClip) if(scr > scrEnd) return ;
         };
     } else if(x+sx < frmWidthEnd) { // clip left only
         u32 lnb, sze ; clDeep *s = &pixel[y*bufWidth + frmWidthStart] ;

         if(s > scrEnd) return ; // out of screen at up
         u32 upClip = (s + sy*bufWidth > scrEnd) ; // is clipped at up ?

          Gfm += 3;

         if(s < scrStart) // is clipped at down ?
         {  if(s + sy*bufWidth < scrStart) return ; // out of screen at down
            do{ lnb = *Gfm++ ;
                while(lnb--) { Gfm++ ; Gfm += *Gfm++ ; };
                s += bufWidth ; sy-- ;
            } while(s < scrStart) ;
         }

         clDeep*scr = s+x ;
         scr -= frmWidthStart ;
         clDeep *screen = scr ;
         u32 c=0 ; u32 size ;

         while(c < sy)
         {  lnb = *Gfm++ ;
            while(lnb--) { scr += *Gfm++ ; sze = *Gfm++ ;
                           if(scr >= s)   memcpy(scr,Gfm,sze<<clDeepDec) ;
                           else if(scr + sze > s) { size = (scr + sze)-s ;
                                                    memcpy(s,Gfm+(sze-size),size<<clDeepDec) ;
                                                  }
                           Gfm += sze ;    scr+=sze ;
            };  scr = screen + bufWidth*(++c) ; s = scr+frmWidthStart ; s -= x ;
            if(upClip) if(s > scrEnd) return ;
         };
     } else return ;
}*/


void udrawGfm(clDeep*Gfm,clDeep*scr)
{
    Gfm+=2 ; u32 sy = (*Gfm++)&65535 ;

		//printf("%u\n",sy);

    u32 lnb, sze ;

    if(scr > scrEnd){
			/*printf("pxl %p\n",(void*)pixel);
			printf("str %p\n",(void*)scrStart);
			printf("scr %p\n",(void*)scr);
			printf("end %p\n",(void*)scrEnd);*/
			//printf("dif %u\n",(scrEnd - scr));
			//printf("out of bound %u\n", scr > scrEnd);
			//printf("up out %u - %u - %u - %ux%u - %u\n",(pixel - scr), (pixel - scrStart), (scrEnd - scrStart),WIDTH,HEIGHT,scr-scrEnd);
			return ;
		} // out of screen at up
    u32 upClip = (scr + sy*bufWidth > scrEnd) ; // is clipped at up ?

    if(scr < scrStart) // is clipped at down ?
    {  //printf("down clip!");
			if(scr + sy*bufWidth < scrStart) return ; // out of screen at down
       do{    lnb = *Gfm++ ;
              while(lnb--) { Gfm++ ; Gfm += *Gfm++ ; };
              scr += bufWidth ; sy-- ;
       } while(scr < scrStart) ;
    }

    clDeep *screen = scr ;
    u32 c = 0 ;

    if(upClip)
     {  //printf("up clip!");
		 while(c < sy)
        {  lnb = *Gfm++ ;
           while(lnb--) { scr += *Gfm++ ; sze = *Gfm++ ;
                          while(sze>1)
                            { *scr++ = *Gfm++;
                              *scr++ = *Gfm++;
                              sze-=2;
                            };
                          if(sze) *scr++ = *Gfm++;
           };  scr = screen + bufWidth*(++c) ;
           if(scr > scrEnd) return ;
        };
     }
    else
     {  while(c < sy)
        {  lnb = *Gfm++ ;
           while(lnb--) { scr += *Gfm++ ; sze = *Gfm++ ;

                          while(sze>1)
                            { *scr++ = *Gfm++;
                              *scr++ = *Gfm++;
                              sze-=2;
                            };
                          if(sze) *scr++ = *Gfm++;
           };  scr = screen + bufWidth*(++c) ;
        };
     }
}


u32 color32(u32 color)
{      /*register u32 r,g,b;
			 r = (color>>16)&0xff;
			 g = (color>>8)&0xff;
			 b = (color)&0xff;
			 return (r<<24)|(g<<16)|(b<<8);*/
			 return color << 8;
}


// convert a 4b Gfm to a 32b Gfm
clDeep * data2Gfm(unsigned char *data)
{ //unsigned char *d = data ;
  data+=3; u32 clNum=*data++ ;
	data+=4 ; // remove for old format
  u32 sx   = *(short*)data ; data+=2 ;
  u32 sy   = *(short*)data ; data+=2 ;
  u32 outSize = *(u32*)data ; data+=4 ;

  //printf("\n%i colors %i*%i out size %i\n",clNum,sx,sy,outSize) ;

	//if( !clNum ) exit(1);//return 0;


  clDeep *Gfm =  (clDeep*)malloc(outSize) ;
  clDeep *pal = clNum ? (clDeep*)malloc(4*clNum) : 0;
  //memcpy(pal,data,4*clNum) ;


	  if( !Gfm || (clNum && !pal) ){
	    printf("malloc error !\n");
	    exit(1);
	  }

	{  u32 *pal32 = (u32*)data;
		 for(u32 n=0;n<clNum;n++)
					 pal[n] = color32( pal32[n] );
	}

	data+=(4*clNum) ;

	clDeep *o = Gfm ;
  u32 cnt=0, c, jump, size, p1, p2 ;





/*
  //printf("\n\nout size : %i\n%i colors { %x",outSize,clNum,*pal) ;
  for(u32 c=1;c<clNum;c++)
	{	//printf(",%x",pal[c]) ;
		u32 color = pal[c];
		u32	r = color & 0xff;,
				g = (color >> 8)  & 0xff,
				b = (color >> 16) & 0xff;
		pal[ c ] =
	}
	//printf(" }\nsize : %i*%i",sx,sy) ;
*/

  *o++ = 0x6d6647 ;              // put signature "Gfm\0"
  *o++ = outSize ;               // put Gfm object size
	//u16 * o16 = (u16*)o++;
  *o++ = sx<<16 | (sy & 65535) ; // put frame size x and y
	//*o16++ = sx;
	//*o16 = sy;

  while(cnt++ < sy)
  {    *o++ = c = *data++ ; //printf("\n* line %i, %i sublines",cnt,c) ;
       while(c--) { jump = *data++ ; size = *data++ ; *o++ = jump ; *o++ = size ;
                    //printf("\n jmp %i sze %i | ",jump,size) ;
                    if(jump > sx || size > sx) {
                      //printf("\njump or size error ... pos %i",data-d) ;
                      //return 0 ;
                    }

                    while(size > 1) { p1 = (*data)>>4 ; p2 = (*data)&15 ;
                                      //printf(",%x,%x",p1,p2) ;
                                      size-=2 ; *o++ = pal[p1]>>8 ; *o++ = pal[p2]>>8 ;

                                      if(p1 >= clNum || p2 >= clNum) {
                                       //printf("\ndata error, out of pal ! ... pos %i ... data %i | %x | %c",data-d,*data,*data,*data) ;
                                        //return 0 ;
                                      }

                                      ++data ;
                                    };
                    if(size!=0) { //printf(" + %x",*data);
									*o++ = pal[(*data++)]>>8 ;
								}
                  };
  };
  *o = 0x2a2a2a2a ;
//  addFreeEntry(Gfm);
  return Gfm ;
}

void unCrunchGfm(clDeep ** gfm, u32 frmNb)
{	for(u32 c=0;c<frmNb;c++)
		gfm[c] = data2Gfm((u8*)(gfm[c]));
}

void flipSprite(clDeep *img, int sx, int sy)
{ clDeep *tmp = (clDeep*)malloc(sx*sy*sizeof(clDeep)) ;
  int ssx = sx-1, ssy = sy-1 ;
  for(int x=0;x<sx;x++)
    for(int y=0;y<sy;y++)
      tmp[(ssy-y)*sx + (ssx-x)] = img[y*sx + x] ;
  memcpy(img,tmp,sx*sy*sizeof(clDeep)) ; free(tmp) ;
}

//#define VERBOSE

clDeep* scanImg(clDeep *img, int sx, int sy, clDeep *trClr)
{   clDeep *start, *lend;
    int tfull=0, ttr=0 ; int line=0 ;
    clDeep *dta  = (clDeep*)malloc((sx*sy+sy*42*6)*sizeof(clDeep)) ;        int size ;
    clDeep * jmp = dta+sx*sy ; clDeep * sze = jmp + sy*42*2 ; clDeep * cln = sze + sy*42*2 ;
    clDeep * jmpPtr = jmp ;    clDeep * szePtr = sze ;    clDeep * dtaPtr = dta ; clDeep * clnPtr = cln ;
    int h=sy ;
    //*trClr = 0xC0C0BC02 ;
    img = img + sx*sy ; img -= sx ;
    lend = img + sx ;
    while(h--)
    {  *clnPtr = 0 ;
     while(img < lend)
     { (*clnPtr)++ ;
       start=img ;
       while(img < lend && *img == *trClr) ++img ;

       size = img-start ;
       ttr+=size ; start=img ; *jmpPtr++ = size ;

       while(img < lend && *img != *trClr) ++img ;

       size = img-start ; *szePtr++ = size ;
       if(size) { memcpy(dtaPtr,start,size*4) ; line++ ; dtaPtr+=size ; }
       else {  (*clnPtr)-- ; szePtr-- ; jmpPtr-- ; }
     }; clnPtr++ ;
         lend -= sx ; img = lend-sx ;
    };

    tfull = dtaPtr-dta ;
    int objectSize = 16 + (sy + line*2 + tfull)*4 ;
    clDeep *out = (clDeep *)malloc(objectSize) ; // alloc size for the Gfm data

    #ifdef VERBOSE
           printf("\nalloc %i bytes for the object.",objectSize) ;
    #endif

    clDeep *o = out ;

    *o++ = 0x6d6647 ;            // put signature "Gfm\0"
    *o++ = objectSize ;            // put Gfm object size
    *o++ = sx<<16 | (sy & 65535) ; // put frame size x and y

    #ifdef VERBOSE
        printf("\n\nscan result\n") ;
    #endif

    int c=0 ; jmpPtr=jmp ; szePtr=sze ; dtaPtr=dta ; u32 cn ;
    while(c < sy)
    {   *o++ = cln[c] ;
        #ifdef VERBOSE
            printf("\nline %i\t %i full lines\t{ ",sy-c,cln[c]) ;
        #endif
        for(cn=0;cn < cln[c];cn++)
        {
          #ifdef VERBOSE
            if(*jmpPtr) printf("+%i ",*jmpPtr) ;
            printf("w%i ",*szePtr) ;
          #endif
          *o++ = *jmpPtr ; *o++ = *szePtr ;
          memcpy(o,dtaPtr,(*szePtr)*4) ; dtaPtr += *szePtr ; o += *szePtr ;
          jmpPtr++ ; szePtr++ ;
        };
        c++ ;
        #ifdef VERBOSE
           printf("}") ;
        #endif
    };      *o++ = 0x2a2a2a2a ; // end check with "****"
        #ifdef VERBOSE
               printf("\n\nobject size : %i bytes", (o-out)*4) ;
               printf("\n\n* total : \ntr   %i\nfull %i\nsum  %i\n%i lines\nsx * sy = %i * %i = %i\n",ttr,tfull,ttr+tfull,line,sx,sy,sx*sy) ;
        #endif
    free(dta) ; return out ;
}

//#undef VERBOSE

clDeep * cropSprite( clDeep * i, int * sx, int * sy, u32 up, u32 down, u32 left, u32 right ){
	int s = *sx * *sy;

/*	printf("crop sprite .. %u x %u\n",*sx,*sy);
	for( int y = 0; y < *sy; y++ ){
		for( int x = 0; x < *sx; x++ ){
			printf("%c", i[ y*(*sx) + x ] == 0xffffffff ? '-' : 'x');
		}
		printf("\n");
	}
*/
	/*for( int y = 0; y < *sy; y++ ){
		for( int x = 0; x < *sx; x++ ){
			printf("%c", i[ y*(*sx) + x ] == 0xffffffff ? '-' : 'x');
		}
		printf("\n");
	}*/

	clDeep * p = i, *end = &p[ s ], *last = end - 1;
	u32 x1 = 0, y1 = 0, x2 = *sx - 1, y2 = *sy - 1;

	while( p != end && *p == 0xffffffff ) p++;
	if( p == end ){ *sx = *sy = 0; return 0; } // full transparent sprite

	if( up ){ // search from top
		//while( p != end && *p == 0xffffffff ) p++;
		//if( p == end ){ *sx = *sy = 0; return 0; } // full transparent sprite
		int size = p - i; while( size >= *sx ){ size -= *sx; y1++; }
		//printf(" top %u\n",y1);
	}

	if(down){ // search from bottom
		p = last;
		while( p >= i && *p == 0xffffffff ) p--;
		int size = last - p; while( size >= *sx ){ size -= *sx; y2--; }
		//printf(" bottom %u\n",y2);
	}

	if( left ){ // search from left
		clDeep * pleft = p = i;
		while( p < end ){
			int found = 0;
			for( int y = 0; y < *sy ; y++ ){ // search colon
				if( *p != 0xffffffff ){ found=1; break; }
				p += *sx;
			};
			if( found ) break;
			p = ++pleft;
		};
		x1 = pleft - i;
		//printf(" left %u\n",x1);
	}

	if( right ){ // search from right
		clDeep * pright = p = &i[ x2  ];
		while( p < end ){
			int found = 0;
			for( int y = 0; y < *sy ; y++ ){ // search colon
				if( *p != 0xffffffff ){ found=1; break; }
				p += *sx;
			};
			if( found ) break;
			x2--;
			p = --pright;
		};
		//printf(" right %u\n",x2);
	}

	// generate out cropped picture
	u32 outsx = (x2+1) - x1, outsy = (y2+1) - y1, lineSize = outsx * sizeof(clDeep);
	clDeep * out = (clDeep*)malloc( outsy * lineSize );

	p = &i[ y1 * *sx + x1 ];
	clDeep *outp = &out[ (outsy - 1)*outsx ];
	for( u32 y = 0; y < outsy; y++ ){
		memcpy( outp, p, lineSize );
		outp -= outsx;
		p += *sx;
	};

	*sx = outsx;
	*sy = outsy;
/*
	printf("%u x %u\n",*sx,*sy);
	for( int y = 0; y < *sy; y++ ){
		for( int x = 0; x < *sx; x++ ){
			printf("%c", out[ y*(*sx) + x ] == 0xffffffff ? '-' : 'x');
		}
		printf("\n");
	}
*/
	return out;
}

clDeep** flipGfm(clDeep **Gfm, u32 nb){
	clDeep ** iGfm = (clDeep**)malloc(sizeof(clDeep*)*nb) ;
	int sx, sy; clDeep tmp ; clDeep *itmp ;
	for(u32 c=0; c<nb; c++){
		tmp = (Gfm[c])[2] ; sx = tmp>>16 ; sy = tmp&65535 ;
		//printf("%u %ux%u\n",c,sx,sy);
		itmp = Gfm2array( Gfm[c] ) ;
		flipSprite(itmp,sx,sy) ;
		iGfm[c] = scanImg(itmp,sx,sy,itmp) ;
		free(itmp) ;
	};
	return iGfm ;
}

clDeep** cropGfm(clDeep **Gfm, u32 nb, u32 up=1, u32 down=1, u32 left=1, u32 right=1){
	clDeep ** cGfm = (clDeep**)malloc(sizeof(clDeep*)*nb) ;
	int sx, sy; clDeep tmp ; clDeep *ctmp ; clDeep trColor = 0xffffffff;
	for(u32 c=0; c<nb; c++){
//		printf("\n%c\n",' '+c);
		tmp = (Gfm[c])[2] ; sx = tmp>>16 ; sy = tmp&65535 ;
		//printf("%u %ux%u\n",c,sx,sy);
		ctmp = Gfm2array(Gfm[c]) ;
		clDeep * cropped = cropSprite(ctmp,&sx,&sy,up,down,left,right) ;
		free( ctmp );
		cGfm[c] = scanImg(cropped,sx,sy,&trColor) ;
		free(cropped) ;
	};
	return cGfm ;
}

void cdrawGfm(clDeep *Gfm)
{
     #ifdef use32b
      int sx = Gfm[2]>>16 ; u32 sy = Gfm[2]&65535 ;
     #else
      int sx = Gfm[0]; int sy = Gfm[1];
     #endif

	 int px = (WIDTH-sx)>>1;
	 int py = (HEIGHT-sy)>>1;

	drawGfm(Gfm,px,py);
}

void udraw4bGfm(u8*Gfm,clDeep*scr,clDeep*mypal){
	Gfm+=3 ;  u32 clNum=*Gfm++ ; Gfm+=4 ;
	u32 sx   = *(u16*)Gfm ; Gfm+=2 ;
	u32 sy   = *(u16*)Gfm ; Gfm+=2 ;
	Gfm+=4 ;
	u32*pal = mypal ? mypal : (u32*)Gfm;
  Gfm+=(4*clNum);

  u32 cnt=0, c, jump, size, p1, p2 ;

  //printf("\n\nout size : %i\n%i colors { %x",outSize,clNum,*pal) ;
  //for(int c=1;c<clNum;c++) printf(",%x",pal[c]) ; printf(" }\nsize : %i*%i",sx,sy) ;

	clDeep * s;

  while(cnt++ < sy)
  {    c = *Gfm++ ; //printf("\n* line %i, %i sublines",cnt,c) ;
       s = scr;
	   while(c--) { jump = *Gfm++ ;
					size = *Gfm++ ;
					scr+=jump;
                    while(size > 1) { p1 = (*Gfm)>>4 ; p2 = (*Gfm)&15 ;
                                      size-=2 ;
									  *scr++ = pal[p1] ; *scr++ = pal[p2] ;
                                      ++Gfm ;
                                    };
                    if(size!=0) { //printf(" + %x",*data);
									*scr++ = pal[*Gfm++] ;
								}
                  };
	   scr = s+WIDTH;
  };
}

/*int Animate(struct anim **b) // check for time and animate if need, not draw
{	struct anim *a = *b ;
	//printf("\nanimate, tick : %u",tick);
	if(a->lastTime + a->frmTime < tick)
	{	//printf("%s","++");
		if(++(a->curentFrm) >= a->frmNumber)
		{	switch(a->animType)
			{ case 0 : a->curentFrm = 0 ; break ; / * loop anim * /
			  case 1 : a->curentFrm = a->frmNumber-1 ; break ; / * stop at last frame * /
			};
			if(a->onfinish) (a->onfinish)(b) ;
		} else if(a->onflip) (a->onflip)(b) ;
		a->lastTime = tick ; return 1 ;
	} else if(a->onplay) a->onplay(b) ;
    return 0 ;
}

void playAnim(struct anim **b,clDeep * screen)//, int way)
{	struct anim *a = *b ;    Animate(b) ;
	udrawGfm((a->Gfm)[a->curentFrm],screen) ;
}

void playAnim(struct anim **b, int x, int y, u32 way)
{   struct anim *a = *b ;    Animate(b) ;
	if(!way) drawGfm((a->Gfm)[a->curentFrm],x,y) ;
     else   drawGfm((a->iGfm)[a->curentFrm],x,y) ;
}

void drawAnim(struct anim **b, int x, int y, u32 way)
{  	struct anim *a = *b ;
    if(!way) drawGfm((a->Gfm)[a->curentFrm],x,y) ;
     else   drawGfm((a->iGfm)[a->curentFrm],x,y) ;
}

void drawFrm(struct anim **b, int x, int y, int frm, u32 way)
{   struct anim *a = *b ;
    if(!way) drawGfm((a->Gfm)[frm],x,y) ;
     else   drawGfm((a->iGfm)[frm],x,y) ;
}

void ifreeAnim(struct anim *a)
{ if(!(a->iGfm)) return ;
  for(u32 c=0;c<a->frmNumber;c++) free((a->iGfm)[c]) ;
}

void mifreeAnim(struct anim **a, u32 nb)
{ for(u32 c=0;c<nb;c++) ifreeAnim(a[c]) ; }

struct anim * setAnim( clDeep **Gfm, u32 nb,
                       u32 frmTime, u32 animType,
                       void (*onfinish)(struct anim**),
                       void (*onflip)(struct anim**),
                       void (*onplay)(struct anim**)
                     )
{	struct anim *a = (struct anim *)malloc(sizeof(struct anim)) ;
	a->Gfm = Gfm ;	a->frmNumber = nb ; //a->iGfm = flipGfm(Gfm,nb) ;
	a->curentFrm = a->lastTime = 0 ;
    a->onplay=onplay ; a->onflip=onflip ; a->onfinish=onfinish ;
	a->frmTime = frmTime ; a->animType = animType ;
    return a ;
}

void resetAnim(struct anim **a)
{ (*a)->curentFrm=0 ; (*a)->lastTime = tick ; }
*/
