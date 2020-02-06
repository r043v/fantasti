
#include "cpupower.h"
#include "pid.h"
/*
available frequency steps:  120 MHz, 240 MHz, 312 MHz, 408 MHz, 480 MHz, 504 MHz, 600 MHz, 648 MHz, 720 MHz, 816 MHz, 912 MHz, 1.01 GHz, 1.10 GHz, 1.20 GHz, 1.30 GHz, 1.40 GHz
available cpufreq governors: conservative userspace powersave ondemand performance schedutil
*/

int * frequencyKHz = 0, frequencyKHzNb = 0;
/*
const int zfrequencyKHz[] = {
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
  1010000,
  1100000,
  1200000,
  1300000,
  1400000
};*/

char ** frequencies = 0;
/*
const char * zfrequencies[] = {
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
  "1.20 GHz",
  "1.30 GHz",
  "1.40 GHz",
  NULL
};*/

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

//const char * getCpuFrequencyCmd[] = { "cat", "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", NULL };
//const char * getCpuGovernorCmd[] = { "cat", "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", NULL };
const char * getCpuAvailableFrequencyCmd[] = { "cat", "/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies", NULL };

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

      // does we got the available frequency list ?
      if( !frequencyKHz ){ // no, load them
        //printf("load available freqs\n");
        char * sf = p_exec( getCpuAvailableFrequencyCmd );
//        printf("%s\n",sf);

        if( !sf ) return -1;

        if( *sf == 'c' ){ // cat: file not found
          free(sf);
          return -1;
        }

        frequencyKHz = (int*)malloc( 1024 * sizeof( int ) );
        int nb = 0;
        char *p = sf, *start = p;
        while( 1 ){
          if( *p == ' ' ){
            *p = 0;
            frequencyKHz[ nb++ ] = atoi( start );
            //printf("%s\n",start);
            p++;
            while( *p == ' ' ) p++;
            start = p;
            continue;
          }

          if( *p == 0 ){
            break;
            /*frequencyKHz[ nb++ ] = atoi( start );
            printf("%s\n",start);
            break;*/
          }

/*          if( *p == '\n' ){
            *p = 0;
            frequencyKHz[ nb++ ] = atoi( start );
            printf("lr %s\n",start);
            break;
          }
*/
          p++;
        };
        free(sf);
        frequencyKHzNb = nb;
        int * f = (int*)realloc( frequencyKHz, nb * sizeof(int) );
        if( f ) frequencyKHz = f;
        frequencies = (char**)malloc( nb*64 + (nb+1) * sizeof(char*) );
        p = (char*)frequencies + (nb+1) * sizeof(char*);
        for( int n=0; n < nb ; n++ ){
          frequencies[n] = p;
          int f = frequencyKHz[n];
          int l;
          if( f > 1000000 ){ // GHz
            float freq = f;
            freq /= 1000000;
            l = sprintf( p, "%1.1f GHz", freq );
          } else { // MHz
            l = sprintf( p, "%i MHz", f/1000 );
          }
          p += 2 + l;
        }
        frequencies[nb]=0;
      }

    struct list * lf = string2List( (const char**)frequencies );
    int fcurrent = -1;

    if( frequency && s == current ){ // already user choice, retreve current freq
      for( int n = 0; n < frequencyKHzNb; n++ ){
      //int n = 0;
      //while(1){
        if( !frequencyKHz[n] ) break;
        //printf("current %i, test %i\n",frequency,frequencyKHz[n]);
        if( frequencyKHz[n] == frequency ){
          //printf("%i match\n",frequency);
          fcurrent = n;
          lf->entries[n].type = 1;
          break;
        }

        if( frequencyKHz[n] > frequency && frequencyKHz[n] - frequency < 50000 ){
          fcurrent = n;
          lf->entries[n].type = 1;
          break;
        }
        //n++;
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
