
#include "cpupower.h"
#include "pid.h"
/*
available frequency steps:  120 MHz, 240 MHz, 312 MHz, 408 MHz, 480 MHz, 504 MHz, 600 MHz, 648 MHz, 720 MHz, 816 MHz, 912 MHz, 1.01 GHz
available cpufreq governors: conservative userspace powersave ondemand performance schedutil
*/

const int frequencyKHz[] = {
  120000,
  240000,
  312000,
  408000,
  480000,
  504000,
  600000,
  648000,
  720000,
  816000,
  912000,
  1010000
};

const char * frequencies[] = {
  "120 MHz",
  "240 MHz",
  "312 MHz",
  "408 MHz",
  "480 MHz",
  "504 MHz",
  "600 MHz",
  "648 MHz",
  "720 MHz",
  "816 MHz",
  "912 MHz",
  "1.01 GHz",
  NULL
};

const char * governors[] = {
  "performance",
  "schedutil",
  "powersave",
  "conservative",
  "userspace",
  "ondemand",
  NULL
};

const char * menu[] = {
  "maximum",
  "adaptive (sched util)",
  "minimum",
  "adaptive (conservative)",
  "user choice",
  "adaptive (on demand)",
  //"shutdown cores",
  NULL
};

//const char * getCpuGovernor = "cpupower frequency-info | grep \"The governor\" | awk '{print $3}' | cut -d\"\\\"\" -f2";
//const char * getCpuFreq = "cpupower frequency-info | grep \"current CPU\" | grep kernel | cut -d\" \" -f6";

const char * getCpuFrequencyCmd[] = { "sudo", "cat", "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq" };
const char * getCpuGovernorCmd[] = { "sudo", "cat", "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor" };
/*
[gs@gs ~]$ sudo cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
schedutil
[gs@gs ~]$ sudo cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq
720000
*/
/*
int getCpuFrequency( void ){
  char * sf = p_exec( getCpuFrequencyCmd );
  int f = atoi( sf );
  free( sf );
  return f;
}

int getCpuGovernor( void ){
  return p_exec( getCpuGovernorCmd );
}

void getPower( void ){
  const char * g[] = { "bash", "-c", getCpuGovernor, NULL };
  const char * f[] = { "bash", "-c", getCpuFreq, NULL };

  char * governor = p_exec( g );
  char * frequency = p_exec( f );

  printf("governor %s\n",governor);
  printf("frequency %s\n",frequency);

  free( governor );
  free( frequency );
}
*/

int cpuCores( void ){
  // get active core number
  const char * nbCmd[] = { "grep", "-c", "^processor", "/proc/cpuinfo", NULL };
  char * snb = p_exec( nbCmd );
  if( !snb ) return -1;
  int nb = atoi( snb ) - 1;
  free( snb );

  const char * s_cores[] = { "1", "2", "3", "4", NULL };
  struct list * l = string2List( s_cores );
  l->entries[ nb ].type = 1;

  int nnb = selectFromList( "active cores", l );
  freeList( l );

  if( nnb == -1 || nnb == nb ) return -1;

  char cmd[256];

  if( nnb > nb ){ // grow
    for( int n = nb+1; n <= nnb; n++ ){
      sprintf( cmd, "echo 1 > /sys/devices/system/cpu/cpu%i/online", n );
      //printf("%s\n",cmd);
      const char * c[] = { "sudo", "su", "-", "-c", cmd, NULL };
      char * r = p_exec( c );
      free( r );
    }

    return -1;
  }

  // reduce
  for( int n = nb; n > nnb; n-- ){
    sprintf( cmd, "echo 0 > /sys/devices/system/cpu/cpu%i/online", n );
    //printf("%s\n",cmd);
    const char * c[] = { "sudo", "su", "-", "-c", cmd, NULL };
    char * r = p_exec( c );
    free( r );
  }

  return -1;
}

int cpuPower( char * governor, int frequency ){
  struct list * l = string2List( menu );

  int current = -1;

  if( governor ){
    current = 0;
    while( 1 ){
      if( !governors[ current ] ){
        current = -1;
        break;
      }

      //printf("g [%s] [%s] %i\n",governors[current], governor, strcmp( governor, governors[current]));

      if( !strcmp( governor, governors[current] ) ){
        //printf("%s match\n",governor);
        l->entries[current].type = 1;
        break;
      }

      current++;
    };
  }

  int s = selectFromList("cpu frequency scaling",l);

  freeList( l );

  if( s == -1 ) return -1;

/*  if( s == 6 ){ // shutdown cores
    // get active core number
    const char * nbCmd[] = { "grep", "-c", "^processor", "/proc/cpuinfo", NULL };
    char * snb = p_exec( nbCmd );
    if( !snb ) return -1;
    int nb = atoi( snb ) - 1;
    free( snb );

    const char * s_cores[] = { "1", "2", "3", "4", NULL };
    struct list * l = string2List( s_cores );
    l->entries[ nb ].type = 1;

    int nnb = selectFromList( "active cores", l );
    freeList( l );

    if( nnb == -1 || nnb == nb ) return -1;

    char cmd[256];

    if( nnb > nb ){ // grow
      for( int n = nb+1; n <= nnb; n++ ){
        sprintf( cmd, "echo 1 > /sys/devices/system/cpu/cpu%i/online", n );
        //printf("%s\n",cmd);
        const char * c[] = { "sudo", "su", "-", "-c", cmd, NULL };
        char * r = p_exec( c );
        free( r );
      }

      return -1;
    }

    // reduce
    for( int n = nb; n > nnb; n-- ){
      sprintf( cmd, "echo 0 > /sys/devices/system/cpu/cpu%i/online", n );
      //printf("%s\n",cmd);
      const char * c[] = { "sudo", "su", "-", "-c", cmd, NULL };
      char * r = p_exec( c );
      free( r );
    }

    return -1;
  }
*/
  if( s == 4 ){ // user choice
    struct list * lf = string2List( frequencies );
    int fcurrent = -1;

    if( frequency && s == current ){ // already user choice, retreve current freq
      for( int n = 0; n < sizeof(frequencyKHz); n++ )
        if( frequency == frequencyKHz[n] ){
          fcurrent = n;
          lf->entries[n].type = 1;
          break;
        }
    }

    int sf = selectFromList("cpu speed", lf);
    freeList( lf );

    if( sf == -1 ) return -1; // back

    if( current != s ){ // not was userspace
      const char * cmd[] = { "sudo", "cpupower", "frequency-set", "-g", "userspace", NULL };
      char * r = p_exec( cmd );
      if( r ) free( r );
    }

    if( fcurrent == sf ) return -1; // already at this freq

    // set freq
    char khz[32];
    sprintf(khz,"%i",frequencyKHz[sf]);
    const char * cmd[] = { "sudo", "cpupower", "frequency-set", "-f", khz, NULL };
    char * r = p_exec( cmd );
    if( r ) free( r );

    //sudo cpupower frequency-set -f 240MHz -g userspace

    return -1;
  }

  if( s == current ) return -1;

  const char * g = governors[ s ];
  if( !g ) return -1;

  // set governor g

  const char * cmd[] = { "sudo", "cpupower", "frequency-set", "-g", g, NULL };
  char * r = p_exec( cmd );

  if( r ) free( r );

  return -1;
}
