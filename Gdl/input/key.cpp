
#ifndef _gdlkey_
#define _gdlkey_

#include "../Gdl.h"

u8 *keyArray = 0 ;
u8 *lastKey = 0 ;

u32 isKeyPress = 0;

void inikey(void)
{	if(!keyArray)
		{	keyArray = (u8*)malloc(keyArraySize*2);
			lastKey = keyArray+keyArraySize;
		}
	memset(keyArray,0,keyArraySize*2);
}

u32 keyUp(u32 key)
{	if(key>keyArraySize) clearLastKey();
	u32 up=0;
	up = (keyArray[key] && lastKey[key]==0);
	lastKey[key] = keyArray[key] ;
	return up;
}

u32 keyRelease(u32 key)
{	if(key>keyArraySize) clearLastKey();
	u32 release=0;
	release = ( keyArray[key] == 0 && lastKey[key] );
	lastKey[key] = keyArray[key] ;
	return release;
}

u32 keyPush(u32 key)
{	if(key>keyArraySize) clearLastKey();
	lastKey[key] = keyArray[key] ;
	return keyArray[key];
}

void clearLastKey( void ){
	memset(lastKey,0xff,keyArraySize);
}

#endif
