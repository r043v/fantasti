/*
available frequency steps:  120 MHz, 240 MHz, 312 MHz, 408 MHz, 480 MHz, 504 MHz, 600 MHz, 648 MHz, 720 MHz, 816 MHz, 912 MHz, 1.01 GHz
available cpufreq governors: conservative userspace powersave ondemand performance schedutil
*/

#ifndef _gdlcpupower_
#define _gdlcpupower_

#include "../Gdl/Gdl.h"
#include "list.h"


int cpuCores( void );
int cpuPower( char * governor, int frequency );
void getPower( void );

#endif
