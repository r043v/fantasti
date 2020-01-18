
/* Gdl² - yAnl */

#include "../Gdl/Gdl.h"

//#include "./gfx/greenfont.h"
//#include "./gfx/bluefont.h"
//#include "./gfx/star.h"

#include "list.h"
#include "pid.h"
#include "explorer.h"
#include "rfkill.h"
#include "cpupower.h"
//#include <zip.h>

/*
u32 Map::set(arDeep*array,clDeep**tileset,
	u32 tileNumber,u32 tileSizex,u32 tileSizey,
	u32 sizex,u32 sizey,u32 scrollx,u32 scrolly,
	outzone*out, u32 copyArray
*/

//arDeep keyboardAarray[  ] =

//Map keyboardMap;

//keyBoard.set( mapArray, tileset, tilesetFrmNb, 32, 32, 24, 64, 0, 0, screen, 0 );


#define POWER_PATH "/sys/class/power_supply/"
#define USB_POWER "axp20x-usb"
#define BAT_POWER "axp20x-battery"
#define USB_POWER_PATH POWER_PATH USB_POWER
#define BAT_POWER_PATH POWER_PATH BAT_POWER

#define CPU_FREQ_PATH "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq"

// get devices > cat /sys/class/power_supply/*/type

struct power {
  struct {
    bool present;
  } usb;
  struct {
    bool present;
    char state[16];
    int capacity;
  } bat;
  struct {
    char governor[32];
    int frequency;
    int frequencyMhz;
  } cpu[4];
  int cpuNumber;
};

struct power power;

struct network {
  char * ssid;
  char * ip;
};

struct network network;

bool updateNetwork( struct network * network ){
  bool update = false;

  const char * getWlanSSID[] = { "bash", "-c", "iw dev | sed -n 's/\\t\\tssid //p'", NULL };
  const char * getWlanIp[] = { "hostname", "-i", NULL };

  char * ssid = p_exec( getWlanSSID );
  char * ip = p_exec( getWlanIp );

  if( ssid ){
    if( !network->ssid || strcmp( ssid, network->ssid ) ){
      if( network->ssid ) free( network->ssid );
      network->ssid = ssid;
      update = true;
    }
  }

  if( ip ){
    if( !network->ip || strcmp( ip, network->ip ) ){
      if( network->ip ) free( network->ip );
      network->ip = ip;
      update = true;
    }
  }

  return update;
}


/*$ pacman -Sl pingu
pingu fantasti-gameshell 0.4.i-1 [installed: 0.4.h-1]
pingu gameshell 0.0.2-1 [installed]
pingu libretro-bsnes-gameshell r1161.309341d0-1
pingu libretro-pocketsnes-gameshell r228.354bcb5-1 [installed]
pingu linux-gameshell 5.4.5-1 [installed]
pingu linux-gameshell-headers 5.4.5-1 [installed]
pingu mesa-lima 20.0.0_devel.1.ce52b49-1 [installed]
pingu monsterwm-gameshell 0.3.f-1 [installed]
*/


struct package {
  char *name, *version;
  int state;
  int whitelist;
};

struct repository {
  char * buffer, *name;
  struct package * packages;
  int nb;

  struct {
    struct package ** packages;
    int nb;
  } installed;

  struct {
    struct package ** packages;
    int nb;
  } available;

  struct {
    struct package ** packages;
    int nb;
  } upgradable;
};

struct list * packages2List( struct package ** packages, int nb ){
	struct list * l = newList( nb, 0 );
	struct entrie *e = l->entries, *last = &l->entries[l->number];

	u32 n = 0;

	while( e < last ){
    struct package * p = packages[n++];
		e->label = p->name;
		e->type = 0;
		e++;
	};

	return l;
}

#define REPO_MAX_PACKAGE_NUMBER 1024
#define REPO_PACKAGE_BUFFER 1024
#define PACKAGE_AVAILABLE 2
#define PACKAGE_INSTALLED 4
#define PACKAGE_UPGRADABLE 8

void readRepository( struct repository * repo, char * name ){
  //printf("update repository database\n");
  // update with pacman
  const char * refresh_cmd[] = { "sudo", "pacman", "-Sy", NULL };
  char *r = p_exec( refresh_cmd );
  if( r ) free( r );

  //printf("get repository packages\n");
  // ask pacman list content of our specific repository
  const char * cmd[] = { "pacman", "-Sl", name, NULL };
  char ** res = pipe_exec( cmd );
  if( !res ) return;

  /*int nnn=0;
  while(1){
    char * s = res[nnn++];
    if( !s ) break;
    printf("[%s]\n",s);
  }*/

  //printf("init repository struct\n");

  // reset repo struct
  //if( repo->buffer ) free( repo->buffer );
  //memset( repo, 0, sizeof( struct repository ) );

  // ask expac to catch our whitelisted packages
  const char * whitelistCmd[] = { "bash", "-c", "expac -S '%D' pingu/gameshell | sed \"s/  /\\n/g\"", NULL };
  char ** whitelist = pipe_exec( whitelistCmd );

  repo->nb = repo->installed.nb = repo->upgradable.nb = repo->available.nb = 0;
  repo->buffer = (char*)malloc( 64 // repo name
  + REPO_MAX_PACKAGE_NUMBER * (
      REPO_PACKAGE_BUFFER // package name, version, description
      + sizeof( struct package * ) * 3 // struct for installed / available / upgradable list links
      + sizeof( struct package ) // package struct
    )
  );
  char *b = repo->buffer;
  r = name;
  repo->name = b;
  while( *b++ = *r++ ); // copy name
  b = repo->buffer + 64;
  repo->installed.packages = (struct package **)b;
  b += REPO_MAX_PACKAGE_NUMBER * sizeof( struct package * );
  repo->available.packages = (struct package **)b;
  b += REPO_MAX_PACKAGE_NUMBER * sizeof( struct package * );
  repo->upgradable.packages = (struct package **)b;
  b += REPO_MAX_PACKAGE_NUMBER * sizeof( struct package * );
  repo->packages = (struct package *)b;
  b += REPO_MAX_PACKAGE_NUMBER * sizeof( struct package );

  // loop around each package & fill our structs

  //printf("loop packages\n");

  while( 1 ){
    struct package * package = &repo->packages[ repo->nb ];
    char * ptr = res[ repo->nb ];
    if( !ptr ) break;
    //printf("[%s]\n",ptr);

    char * p = ptr;
    // repo name, skip
    while( *p && *p != ' ' ) p++;
    if( !*p++ ) break;

    // copy package name
    package->name = b;
    while( *p && *p != ' ' ) *b++ = *p++;
    *b++ = 0;
    if( !*p++ ) break;

    int n = 0;
    package->whitelist = 0;
    //printf("-- %s --\n", package->name);
    while( whitelist[n] ){
      char *w = whitelist[n], *p = package->name;
      while( *w && *w != '\n' && *p && *p == *w ){
        w++; p++;
      }
      if( !*p && *w == '\n' ){
      //if( !strcmp( whitelist[n], package->name ) ){
        package->whitelist = 1;
        //printf("%s match\n",whitelist[n]);
        break;
      }
      //printf("%s not match\n",whitelist[n]);
      n++;
    };
    if( !package->whitelist && !strcmp( "gameshell", package->name ) ) package->whitelist = 1;

    //printf("whitelist ? %i\n",package->whitelist);

    //printf("%s\n",package->name);

    // copy package version
    package->version = b;
    while( *p && *p != ' ' ) *b++ = *p++;
    *b++ = 0;

    //printf("%s\n",package->version);

    /* all is ok, increment packages number */
    repo->nb++;

    // check package state
    if( !*p++ ){ // package is not installed
      package->state = PACKAGE_AVAILABLE;
      if( !package->whitelist )
        repo->available.packages[ repo->available.nb++ ] = package;
      continue;
    }

    // package is installed
    package->state = PACKAGE_INSTALLED;
    if( !package->whitelist )
      repo->installed.packages[ repo->installed.nb++ ] = package;

    p++;
    while( *p && *p != ' ' ) p++;

    if( *p == ' ' ){ // package can be updated
      package->state &= PACKAGE_UPGRADABLE;
      repo->upgradable.packages[ repo->upgradable.nb++ ] = package;
    }
  };

  free( whitelist );

  // end of lists
  //repo->packages[ repo->nb ] = NULL;
  repo->installed.packages[ repo->installed.nb ] = NULL;
  repo->available.packages[ repo->available.nb ] = NULL;
  repo->upgradable.packages[ repo->upgradable.nb ] = NULL;

  free( res );
}

bool updateCpu( struct power * power ){
  const char * getCpu0[] = { "sudo", "cat", "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", NULL };
  const char * getCpuFrequencyCmd[] = { "sudo", "bash", "-c", "cat /sys/devices/system/cpu/cpu?/cpufreq/scaling_cur_freq", NULL };
  const char * getCpuGovernorCmd[] = { "sudo", "cat", "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", NULL };

  bool update = false;

  char ** freqs = pipe_exec( getCpu0/*FrequencyCmd*/ );
  int cpu = 0;
  while( freqs[cpu] ){
    int f = atoi( freqs[cpu] );

    if( power->cpu[cpu].frequency != f ){
      update = true;
      power->cpu[cpu].frequency = f;
      power->cpu[cpu].frequencyMhz = f/1000;
    }

    //printf( "cpu %i %iMhz\n",cpu,f );

    cpu++;
  }
  power->cpuNumber = cpu;
  free( freqs );

/*  char *sf = p_exec( getCpuFrequencyCmd );
  char *p = sf, *start = p;
  int cpu = 0;
  printf("%s",sf);

  if( sf && *sf ){
    while( 1 ){
      bool end = !*p;
      if( *p == '\n' || !*p ){
        *p = 0;
        int f = atoi( start );

        if( power->cpu[cpu].frequency != f ){
          update = true;
          power->cpu[cpu].frequency = f;
          power->cpu[cpu].frequencyMhz = f/1000;
        }

        printf( "cpu %i %iMhz\n",cpu,f );

        cpu++;
        start = p+1;
      }

      if( end ) break;

      p++;
    };
//  int f = atoi( sf );
    free( sf );
  }*/
/*  if( power->cpu.frequency != f ){
    update = true;
    power->cpu.frequency = f;
    power->cpu.frequencyMhz = f/1000;
  }*/

  char * g = p_exec( getCpuGovernorCmd );
  if( g && strcmp( power->cpu[0].governor, g ) ){
    update = true;
    strncpy( power->cpu[0].governor, g, 31 );
    //printf("%s\n",g);
  }

  if( g ) free( g );

  return update;
}

bool confirm( void ){
  const char * str[] = { "yes", "no", NULL };
  struct list * l = string2List( str );
  int s = selectFromList("confirm ?", l);
  freeList(l);
  do { Gdl_updateMsg(); Gdl_Sleep(1); } while( keyPush(kenter) );
  return s == 0;
}

bool updatePower( struct power * power ){
  bool update = false;

//  getPower();

  FILE * f;

  // get cpu frequency

  // check bat present
  if( f = fopen( BAT_POWER_PATH "/present" , "r") ){
    bool p = '1' == fgetc( f );
    if( power->bat.present != p ){
      update = true;
      power->bat.present = p;
    }
    fclose( f );
  }

  // check usb present
  if( f = fopen( USB_POWER_PATH "/present" , "r") ){
    bool p = '1' == fgetc( f );
    if( power->usb.present != p ){
      update = true;
      power->usb.present = p;
    }
    fclose( f );
  }

  // check battery capacity
  if( f = fopen( BAT_POWER_PATH "/capacity" , "r") ){
    char b[4];
    size_t l = fread( b, 1, 4, f );
    int value = power->bat.capacity;
    if( l <= 0 )
      power->bat.capacity = -1;
    else {
      b[l] = 0; power->bat.capacity = atoi( b );
    }
    if( value != power->bat.capacity ) update = true;
    fclose( f );
  }

  // check battery state
  if( f = fopen( BAT_POWER_PATH "/status" , "r") ){
    char state = *power->bat.state;
    size_t l = fread( power->bat.state, 1, 15, f );
    if(l) l--; power->bat.state[ l ] = 0;
    if( state != *power->bat.state ) update = true;
    fclose( f );
  }

  return update;
}
// ⚡
#define AC_POWER_LOGO "⚡"
#define BAT_POWER_LOGO " "

void printPower( void ){
    if( !power.bat.present ) return;
    fprints( power.bat.capacity == 100 ? 276 : 282, 0, "%s%i%%", /*power.usb.present*/ ( !strcmp("Charging", power.bat.state ) ) ? AC_POWER_LOGO : BAT_POWER_LOGO, power.bat.capacity );
//    fprints( 192, 0, "%s", power.bat.state );
}

void printNetwork( void ){
  if( !network.ssid || !*network.ssid )
    fprints( 4, 220, "no network" );
  else
    fprints( 4, 220, "%s / %s", network.ssid, network.ip);//, power.cpu[0].frequencyMhz);//, power.cpu[1].frequencyMhz, power.cpu[2].frequencyMhz, power.cpu[3].frequencyMhz );
}

struct freetypeFont * ftFont ;
struct explorer * xHome = 0;

struct entrieCommand entryCmds[] = {

		{ "explorer", [] () {
      if( !xHome ) xHome = explorer( getenv("HOME" ) );
			showExplorer( xHome );// getenv("HOME" ) );
      if( xHome->length == 1 ){
        freeExplorer( xHome );
        xHome = 0;
      }

			return 0;
		} },

    { "shutdown", [] () {
      const char * cmd[] = { "sudo", "shutdown", "now", NULL };
      _exec( cmd, 0, 0 );

      return -1;
    } },

    { "process", [] () {
        struct list * l = pidStack2list();

        const char *cmds[] = { "focus", "terminate", "kill", NULL };
        struct list * pcmds = string2List( cmds );

        char *shellCmds[] = {
          "xdotool windowactivate `xdotool search --pid %s | tail -1`",
          "kill %s",
          "kill -9 %s"
        };

        while( 1 ){
          int app = selectFromList("process list",l);
          if( app == -1 ) break;
          int action = selectFromList( "action", pcmds );
          if( action == -1 ) continue;
          char pid[8], *p = pid;
          //u32 pid = 0;
          //printf("app %i\n",app);
          struct entrie * e = &l->entries[ app ];
          if( !e ) break;
          const char * pl = e->label;
          //printf("entrie %s\n",pl);
          while( *pl && *pl != ' ' ) *p++ = *pl++;
          *p = 0;
          //printf("pid %s\n",pid);
          char shellCmd[1024];
          sprintf( shellCmd, shellCmds[ action ], pid );
          //printf("cmd %s\n",shellCmd);
          const char *bashCmd[] = { "bash", "-c", shellCmd, NULL };

          char * result = p_exec( bashCmd );
          if( result ) free( result );

          if( action > 0) break;

          //freeList( l );
          //l = pidStack2list();
        };

        freeList( l );
        freeList( pcmds );
        return 0;
      }
    },

    { "reboot", [] () {
      const char * cmd[] = { "sudo", "reboot", "now", NULL };
      _exec( cmd, 0, 0 );

      return -1;
    } },

    { "cpu power", [] (){
      updateCpu( &power );
			cpuPower( power.cpu[0].governor, power.cpu[0].frequency );
			return 0;
		} },

    { "network", [] (){
				while( networkMenu() != -1 );
				return 0;
		} },

    { "cpu cores", [] (){
      cpuCores();
      return 0;
    } },

    { "htop", [] () {
      const char * cmds[] = { "xterm", "-e", "htop", NULL };
      _exec( cmds, 1, 0 );
      return 0;
    } },

    { "retroarch", [] (){ exec("retroarch"); return 0; } },

    { "xclock", [] (){ exec("xclock"); return 0; } },

    { "retroarch fbneo", [] () {

      const char * cmd[] = { "retroarch", "-L", "/usr/lib/libretro/fbneo_libretro.so", "--menu", NULL };
      _exec( cmd, 1, 0 );

      return 0;
    } },

//		{ "xterm", [] (){ exec("xterm"); return 0; } },

    { "packages", [] (){
      struct repository pingu;
      readRepository( &pingu, "pingu" );
      char *bf = (char*)malloc( 256 ), *availableStr = bf, *installedStr = bf + 64, *upgradableStr = bf + 128, *titleStr = bf + 192;

      u32 actionNb = 0;
      const char *actionStrings[8] = { };//, "availables", "installed", "upgradable", NULL };

      if( pingu.available.nb ){
        sprintf( availableStr, pingu.available.nb == 1 ? "%s, %i %s " : "%s, %i %ss", "install", pingu.available.nb, "available" );
        actionStrings[ actionNb++ ] = (const char *)availableStr;
      } else actionStrings[actionNb++] = "nothing to install";

      if( pingu.installed.nb ){
        sprintf( installedStr, pingu.installed.nb == 1 ? "%i %s " : "%i %ss", pingu.installed.nb, "installed package" );
        actionStrings[ actionNb++ ] = (const char *)installedStr;
      } else actionStrings[actionNb++] = "nothing installed";

      if( pingu.upgradable.nb ){
        sprintf( upgradableStr, pingu.upgradable.nb == 1 ? "%i %s %s" : "%i %ss %s", pingu.upgradable.nb, "package", "upgradable" );
        actionStrings[ actionNb++ ] = (const char *)upgradableStr;
      } else actionStrings[actionNb++] = "nothing to upgrade";

      if( pingu.upgradable.nb ){
        actionStrings[ actionNb++ ] = "upgrade all packages";
      }

      actionStrings[ actionNb ] = NULL;

      sprintf(titleStr,"%s repository, %i packages", pingu.name, pingu.nb);

      struct list * actions = string2List( actionStrings );

      while(1){
        int action = selectFromList( titleStr, actions );
        if( action == -1 ){
          break;
        }

        if( action == 0 ){ // install
          if( !pingu.available.nb ) continue;
          struct list * l = packages2List( pingu.available.packages, pingu.available.nb );
          int p = selectFromList( availableStr, l );
          freeList( l );
          if( p == -1 ) continue;
          struct package * package = pingu.available.packages[ p ];
          const char * actionStrings[] = { "install", /*"install (no confirm)", "install (overwrite)",*/ NULL };
          struct list * actions = string2List( actionStrings );
          char * title = (char*)malloc(2048);
          sprintf( title, "%s %s", package->name, package->version );
          int a = selectFromList( title, actions );
          free(title);
          freeList(actions);
          if( action == -1 ) continue;
          if( !confirm() ) continue;

          const char * cmds[] = {
            "sudo pacman --color=always -S %s %s",
            "sudo pacman --color=always -S %s --noconfirm %s",
            "sudo pacman --color=always -S %s --overwrite='*' %s"
          };

          char * bf = (char *)malloc( 4096 );
          sprintf(bf, cmds[action], package->name, "&& read -p \"Press enter to continue\"" );

          const char * cmd[] = {
            "xterm", "-e", bf, NULL
          };

          _exec( cmd, 0, 0 );

          free(bf);
          break;
        }

        if( action == 1 ){ // installed
          if( !pingu.installed.nb ) continue;
          struct list * l = packages2List( pingu.installed.packages, pingu.installed.nb );
          int p = selectFromList( installedStr, l );
          freeList( l );
          if( p == -1 ) continue;

          struct package * package = pingu.installed.packages[ p ];
          const char * actionStrings[] = { "uninstall", /*"uninstall (no confirm)",*/ NULL };
          struct list * actions = string2List( actionStrings );
          char * title = (char*)malloc(2048);
          sprintf( title, "%s %s", package->name, package->version );
          int a = selectFromList( title, actions );
          free(title);
          freeList(actions);
          if( action == -1 ) continue;
          if( !confirm() ) continue;

          const char * cmds[] = {
            "sudo pacman --color=always -R %s && read -p \"Press enter to continue\""//,
            //"sudo pacman --color=always -R %s --noconfirm %s"
          };

          char * bf = (char *)malloc( 4096 );
          sprintf(bf, cmds[action], package->name);//, "&& read -p \"Press enter to continue\"" );

          const char * cmd[] = {
            "xterm", "-e", bf, NULL
          };

          _exec( cmd, 0, 0 );

          free(bf);

          break;
        }

        if( action == 2 ){ // update
          if( !pingu.upgradable.nb ) continue;
          struct list * l = packages2List( pingu.upgradable.packages, pingu.upgradable.nb );
          int p = selectFromList( upgradableStr, l );
          freeList( l );
          if( p == -1 ) continue;
          if( !confirm() ) continue;

          struct package * package = pingu.upgradable.packages[ p ];
          char * bf = (char *)malloc( 4096 );
          sprintf(bf, "sudo pacman --color=always -S %s && read -p \"Press enter to continue\"", package->name );

          const char * cmd[] = {
            "xterm", "-e", bf, NULL
          };

          _exec( cmd, 0, 0 );

          free(bf);

          break;
        }

        if( action == 3 ){ // upgrade all
          const char * cmds[] = { "xterm", "-e", "echo \"update pingu repository\"; sudo pacman --color=always -Sy && ( p=$(pacman -Sl pingu | grep \"installed:\" | cut -d' ' -f2 | tr '\\n' ' ') && [ -z \"$p\" ] && echo \"system is up to date\" || sudo pacman --color=always -S $(echo \"$p\") ); read -p \"press enter to continue\"", NULL }; // --noconfirm
          _exec( cmds, 0, 0 );
          break;
        }
      }

      freeList( actions );
      free( bf );

      return 0;

/*$ pacman -Sl pingu
pingu fantasti-gameshell 0.4.i-1 [installed: 0.4.h-1]
pingu gameshell 0.0.2-1 [installed]
pingu libretro-bsnes-gameshell r1161.309341d0-1
pingu libretro-pocketsnes-gameshell r228.354bcb5-1 [installed]
pingu linux-gameshell 5.4.5-1 [installed]
pingu linux-gameshell-headers 5.4.5-1 [installed]
pingu mesa-lima 20.0.0_devel.1.ce52b49-1 [installed]
pingu monsterwm-gameshell 0.3.f-1 [installed]
*/

// pacman -Sl pingu | grep -v "\[installed\]$" | cut -d" " -f2 > /tmp/available
// pacman -Sl pingu | grep "\[installed\]$" | cut -d" " -f2 > /tmp/installed
// pacman -Sl pingu | cut -d" " -f2 > /tmp/all
// expac -S '%D' pingu/gameshell | sed "s/  /\n/g" > /tmp/whitelist
// grep -v -f /tmp/whitelist /tmp/installed | grep -vx "^gameshell"

// pacman -Sl pingu | grep "\[installed\]$" | cut -d" " -f2 > /tmp/installed

// sed -n '/pattern/!{p;d}; w file_1' input.txt > file_2

//pacman -Sl pingu | grep -v "\[installed\]$" | cut -d" " -f2 > /tmp/available

    } },

/*
    { "update", [] () {
      //const char * cmds[] = { "xterm", "-e", "echo \"update pingu repository\" && sudo pacman -Sy && sudo pacman -S gameshell --noconfirm --needed && sudo pacman -S --needed --noconfirm $(sudo pacman -Sg gameshell) && read -p \"Press enter to continue\"", NULL }; // --noconfirm
      //const char * cmds[] = { "xterm", "-e", "echo \"update pingu repository\" && sudo pacman --color=always -Sy && sudo pacman --color=always -S --needed $(paclist pingu | cut -f 1 -d \" \") && read -p \"Press enter to continue\"", NULL }; // --noconfirm
      // sudo pacman -S --needed $(paclist pingu | cut -f 1 -d " ")
      // pacman -Sl pingu | grep "installed:" | cut -d' ' -f2
      // p=$(pacman -Sl pingu | grep "installed:" | cut -d' ' -f2) && [ -z "$p" ] && echo "system is up to date" || sudo pacman --color=always -S "$p"
// ( p=$(pacman -Sl pingu | grep "installed:" | cut -d' ' -f2) && [ -z "$p" ] && echo "system is up to date" || sudo pacman --color=always -S "$p" ) && read -p "press enter to continue"
      const char * cmds[] = { "xterm", "-e", "echo \"update pingu repository\" && sudo pacman --color=always -Sy && ( p=$(pacman -Sl pingu | grep \"installed:\" | cut -d' ' -f2) && [ -z \"$p\" ] && echo \"system is up to date\" || sudo pacman --color=always -S \"$p\" ) && read -p \"press enter to continue\"", NULL }; // --noconfirm
      _exec( cmds, 0, 0 );
      return 0;
    } },
*/
    // restart X

		{ NULL, NULL }
};

//clDeep ** croppedFont = 0;

// uncrunch data...
void uncrunchData(void)
{	// uncrunch gfx from 4b gfm to 32b gfm
	//unCrunchGfm(greenfont,greenfontFrmNb);
	//unCrunchGfm(bluefont,bluefontFrmNb);
/*
	clDeep ** f = cropGfm( bluefont, bluefontFrmNb, 0,0,1,1 );

	for( u32 n=0; n<bluefontFrmNb; n++ ){
		free( bluefont[n] );
		bluefont[n] = f[n];
	}
*/
/*	clDeep ** f = cropGfm( greenfont, greenfontFrmNb, 0,0,1,1 );

	for( u32 n=0; n<greenfontFrmNb; n++ ){
		free( greenfont[n] );
		greenfont[n] = f[n];
	}
*/
	//croppedFont = cropGfm( bluefont, bluefontFrmNb, 0,0,1,1 );
	//greenfont = cropGfm( greenfont, greenfontFrmNb, 0,0,1,1 );
}

char * font8x8;

clDeep * clDeepSet( clDeep * p, clDeep value, u32 nb ){
	clDeep *end = &p[ nb ];
	while( p != end ) *p++ = value;
	return end;
}

void drawBg( void ){
	clDeep * p = pixel;
	p = clDeepSet(p, 0xcff7f7, 14*SCREEN_WIDTH);
	p = clDeepSet(p, 0, SCREEN_WIDTH);
	p = clDeepSet(p, 0xcff79f, 185*SCREEN_WIDTH);
	p = clDeepSet(p, 0, SCREEN_WIDTH);
	clDeepSet(p, 0xcff7f7, 39*SCREEN_WIDTH);

  printNetwork();
  printPower();
}

extern char * wlanDevice;

int needUpdate = true;

void onUpdate( void ){
  if( !isAppActive ){ Gdl_Sleep(10); return; }
  //printf("%u\n",tick);
  static u32 nextPowerCheck = tick+1000, nextNetworkCheck = tick;//, nextCpuCheck = tick;

  if( nextPowerCheck < tick ){
    nextPowerCheck = tick + 3000;
    needUpdate |= updatePower( &power );
  }

  if( nextNetworkCheck < tick ){
    nextNetworkCheck = ( ( network.ip && *network.ip ) ? 10000 : 3000 ) + tick;
    needUpdate |= updateNetwork( &network );
  }
/*
  if( nextCpuCheck < tick ){
    nextCpuCheck = tick + 2000;
    needUpdate |= updateCpu( &power );
  }
*/
}

int main(int argc, char const *argv[]) {
	Gdl_init( "[fantasti]", SCREEN_WIDTH, SCREEN_HEIGHT );   // init the framebuffer

	uncrunchData();
	//setGdlfont(greenfont);

  initFreetype();
  ftFont = loadFreetypeFont( "/usr/local/share/fantasti/font.ttf" , 12 );
	setFallbackFont(ftFont, "/usr/local/share/fantasti/fallback.ttf");
	setFreetypeColor( ftFont, 33,33,33, 32 );
	setFreetypeFont( ftFont );

  Gdl_iniCallback(
    0, // key
    0, // click
    0, // move
    0, // exit
    onUpdate, // update
    0, // whell
    0, // focus
    0, // drop
    0 // zoom
  );

	struct list * cmdsList = entrieCommands2List( entryCmds );
	while( 0 != showEntrieCommandsList( "fantasti.α ~", entryCmds, cmdsList ) );

  freeList(cmdsList);

	return 0;
}
