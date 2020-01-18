
#include "../Gdl/Gdl.h"
#include "rfkill.h"
#include "pid.h"
#include <iwlib.h>

char wlanDevice[32] = { 0 };
//char wlanDevice[32] = "wlan0";//"wlp3s0";

// get device >>> iw dev | sed -n 's/\tInterface //p'

int rfkill_fd = -1;

void closeRfkill(void){
  if( rfkill_fd == -1 ) return;
//  printf("closing rfkill link..\n");
  close(rfkill_fd);
  rfkill_fd = -1;
}

int openRfkill( void ){
  if( rfkill_fd < 0 ){
    if((rfkill_fd = open("/dev/rfkill", O_RDWR)) < 0){
      printf("error while open rfkill %i\n", rfkill_fd);
      return 0;
    }
    fcntl(rfkill_fd, F_SETFL, O_NONBLOCK);
  }

  return 1;
}

struct entrieCommand networkCmds[] = {
    { "switch off Wifi", [] (){ /*printf("wifi off\n");*/ setRfkill(RFKILL_TYPE_WLAN,1); return -1; } },
    { "switch on Wifi",  [] (){ /*printf("wifi on\n");*/  setRfkill(RFKILL_TYPE_WLAN,0); return -1; } },
    { "switch off Bluetooth", [] (){ /*printf("bt off\n");*/ setRfkill(RFKILL_TYPE_BLUETOOTH,1); return -1; } },
    { "switch on Bluetooth",  [] (){ /*printf("bt on\n");*/  setRfkill(RFKILL_TYPE_BLUETOOTH,0); return -1; } }
};

int wifiScan( void ){

  if( !*wlanDevice ){ // first check
      const char * c[] = { "bash", "-c", "iw dev | sed -n 's/^\\tInterface //p'", NULL };
      //printf("exec %s %s %s\n",*c,c[1],c[2]);
      char * interface = p_exec( c ), *i = interface;
      if( i && *i ){
        //printf("%s\n",i);
        char * w = wlanDevice;
        while( *i && *i != ' ' )
          *w++ = *i++;
        *w=0;
        free( interface );
      }
  }

  //printf("scanning wifi ...\n");
/*	iwrange range;
	int sock = iw_sockets_open();

	if (iw_get_range_info(sock, wlanDevice, &range) < 0) return -1;

	wireless_scan_head head;
	wireless_scan *result;
  u32 scan = 0;

  if( iw_scan(sock, wlanDevice, range.we_version_compiled, &head) < 0 ) return -1;

  char * labels = (char*)malloc(1024), *l = labels;
  char * label[64];
  u32 nb = 0;

//    printf("scan %u\n",scan++);
  result = head.result;
  while( result != NULL ){
    label[ nb++ ] = l;
    char * p = result->b.essid;
    while( *l++ = *p++ );
    //printf("ESSID: %s\n", result->b.essid);
    wireless_scan *next = result->next;
    free( result );
    result = next;
  };

  iw_sockets_close(sock);

  label[ nb ] = NULL;
*/

  const char * wifiScan[] = { "sudo", "fantasti-wl-scan", wlanDevice, NULL };

  char ** wifiList = pipe_exec( wifiScan );

  char * labels = (char*)malloc(2048), *l = labels;
  char * label[256];

  u32 nb=0;
  label[ nb++ ] = (char*)"hidden network";
  while( wifiList[nb] ){
    char * p = wifiList[nb];
    label[ nb++ ] = l;
    while( *p && *p != '\n' ) *l++ = *p++;
    *l++ = 0;
  };

  label[nb] = NULL;
  free( wifiList );

  struct list * wifi = string2List( (const char**)label );

  const char *currentCmd[] = { "bash", "-c", "wpa_cli -i wlan0 status | sed -n 's/^ssid=//p'", NULL };
  char * current = p_exec( currentCmd );
  int currentId = -1;

  if( current ){
    for( u32 n = 0 ; n < wifi->number ; n++ ){
      if( !strcmp( wifi->entries[ n ].label, current ) ){
        wifi->entries[ n ].type = 1;
        currentId = n;
        break;
      }
    };

    free( current );
  }

  u32 network = selectFromList( "Select access point", wifi );
  freeList( wifi );

  if( network == -1 ){
    free( labels );
    return -1;
  }

  char * ssid = label[network];

  if( network == 0 ){ // hidden network
    ssid = askUser( "ssid : " );
    if( !ssid ){ free( labels ); return -1; }
    if( !*ssid ){ free( ssid ); free( labels ); return -1; }
  }

  printf("network %s selected\n", ssid);

  char * password = askUser( "password : " );

  if( !password ){ free( labels ); return -1; }
  if( !*password ){
    free( password );
    free( labels );
    return -1;
  }

  u32 length = strlen(password);
  if( length < 8 || length > 64 ){
    free( password );
    free( labels );
    return -1;
  }

  char * sed = (char*)malloc( 2048 );
  if( network ){
    sprintf( sed, "sed -e \"s/mykey/%s/\" -e \"s/myessid/%s/\" \"/etc/netctl/examples/template\" > \"/etc/netctl/%s\" ; netctl-auto enable %s", password, ssid, ssid, ssid, ssid );
  } else {
    sprintf( sed, "sed -e \"s/mykey/%s/\" -e \"s/myessid/%s/\" -e \"\\$aHidden=yes\" \"/etc/netctl/examples/template\" > \"/etc/netctl/%s\" ; netctl-auto enable %s", password, ssid, ssid, ssid, ssid );
  }

  const char *cmd[] = {
    "sudo", "bash", "-c", sed, NULL
  };

  _exec( cmd );

  if( !network ) free( ssid );
  free( password );
  free( labels );

  return -1;
}

/*      		if (result->b.has_key) {
      			iw_print_key(buffer, sizeof(buffer), result->b.key, result->b.key_size,
      				     result->b.key_flags);
      			printf("key: %s\n", buffer);
      		}
      		if (result->has_ap_addr) {
      			printf("sawap: %s\n", iw_sawap_ntop(&result->ap_addr, buffer));
      		}
      		if (result->b.has_freq) {
      			iw_print_freq_value(buffer, sizeof(buffer), result->b.freq);
      			printf("freq: %s\n", buffer);
      		}
      		if (result->has_maxbitrate) {
      			iw_print_bitrate(buffer, sizeof(buffer), result->maxbitrate.value);
      			printf("bitrate: %s\n", buffer);
      		}
      		if (result->has_stats) {
      			iw_print_stats(buffer, sizeof(buffer), &result->stats.qual, &range, 1);
      			printf("stats: %s\n", buffer);
      		}
      		printf("------------------\n");
*/
/*
  struct list * netctlList( void ){
    const char * cmd[] = { "netctl", "list", NULL };
    struct list * l = newList( 0, 0 );
    char *b = l->labels = (char *)malloc( 1024 );

    void (*cb)(char * s) = [&]( char * s ){
      if( !s ) return;
      struct entrie *e = appendList( l, 1 );
      e->label = b;
      while( *b == ' ' ) b++;
      while( *s && *s != '\n' && *s != ' ' ) *b++ = *s++;
      *b++ = 0;
    };

    callback_exec( cmd, cb );

    return l;
  }
*/

  struct list * netctlList( void ){
    const char * cmd[] = { "netctl", "list", NULL };
    struct list * l = newList( 0, 0 );
    char *b = l->labels = (char *)malloc( 1024 );

    char ** networks = pipe_exec( cmd );

    u32 n = 0;
    while( 1 ){
      char * s = networks[ n ];
      if( !s ) break;
      struct entrie *e = appendList( l, 1 );
      e->label = b;
      e->type = 0;
      while( *s == ' ' ) s++;
      while( *s && *s != '\n' && *s != ' ' ) *b++ = *s++;
      *b++ = 0;
      n++;
    };

    free( networks );

    return l;
  }

struct entrieCommand currentNetworkCmds[] = {
  { 0 },
  { 0 },
  { "wifi scan", wifiScan },
  { "know wifi", [](){
    const char *currentCmd[] = { "bash", "-c", "wpa_cli -i wlan0 status | sed -n 's/^id_str=//p'", NULL };
    char * current = p_exec( currentCmd );
    int currentId = -1;

    struct list * l = netctlList();

    if( current ){
      for( u32 n = 0 ; n < l->number ; n++ ){
        if( !strcmp( l->entries[ n ].label, current ) ){
          l->entries[ n ].type = 1;
          currentId = n;
          break;
        }
      };

      free( current );
    }

    int profile = selectFromList( "know profiles", l );

    if( profile == -1 ){ // back
      freeList(l); return -1;
    }

/*    if( profile == currentId ){
      printf("selected menu\n");
    } else {
      printf("unselected menu\n");
    }*/


    const char * cmds[] = { "enable", "disable", "delete", NULL };
    struct list * c = string2List( cmds );
    int a = selectFromList("action", c );
    freeList(c);

    if( a == -1 ){ freeList(l); return -1; }

    if( a < 2 ){
      const char * shellCmd[] = { "sudo", "netctl-auto", cmds[ a ], l->entries[ profile ].label, NULL };
      char * result = p_exec( shellCmd );
      if( result ) free( result );
    } else { // delete
      char bf[256];
      sprintf(bf, "/etc/netctl/%s",l->entries[ profile ].label );
      const char * shellCmd[] = { "sudo", "rm", "-f", bf, NULL };

      char * result = p_exec( shellCmd );
      if( result ) free( result );
    }

    freeList(l);
    return -1;
  } },
  { "enable usb ethernet", [](){
    const char * cmd[] = { "sudo", "ifconfig", "usb0", "10.0.1.1", "netmask", "255.255.255.0", "up", NULL };
    char * r = p_exec( cmd );
    if( r ) free( r );
    return -1;
  } },
  { NULL, NULL }
};

void readRfkillEvent( struct rfkill_event * event ){
//  printf("id %u type %u op %u soft %u hard %u\n", event->idx, event->type, event->op, event->soft, event->hard);
  u32 type = event->type;
  if( !type || type > 2 ) return; // no wifi or bluetooth
  type -= 1;
  u32 fn = type * 2 + event->soft;
//  printf("< rfkill | operation %u on type %s (%u) state %s fn#%u\n",event->op, rfkillTypes[event->type], type, event->soft ? "off" : "on", fn);

  struct entrieCommand * c = &networkCmds[ fn ];
  struct entrieCommand * e = &currentNetworkCmds[ type ];

//  memcpy( e, c, sizeof( struct entrieCommand ) );
  e->label = c->label;
  e->fn = c->fn;
}

int setRfkill( u8 device, u8 blocked ){
  if( !openRfkill() ) return 0;
  struct rfkill_event event = { 0 };
  event.type = device;
  event.op = RFKILL_OP_CHANGE_ALL;
  event.soft = blocked;

//  printf("set rfkill device %u %s with %u\n",device,rfkillTypes[device],blocked);

  int ok = write(rfkill_fd, &event, sizeof(event)) >= 0;

  while( read( rfkill_fd, &event, sizeof( struct rfkill_event ) ) == RFKILL_EVENT_SIZE_V1 ){
    readRfkillEvent( &event );
  };

  return ok;
}

int networkMenu( void ){
  if( !openRfkill() ) return -1;

  struct rfkill_event event;
  while( read( rfkill_fd, &event, sizeof( struct rfkill_event ) ) == RFKILL_EVENT_SIZE_V1 ){
    readRfkillEvent( &event );
  };

  struct list * l = entrieCommands2List( currentNetworkCmds );
  static int selected = 0;
  l->selected = selected;

  while( 1 ){
    selected = selectFromList( "network control", l );
    if( selected == -1 ){ selected = 0; return -1 ; } // escape
    //printf("user selection %i %s\n", selected, currentNetworkCmds[ selected ].label);
    //memset( pixel, 0, 320*240*4 ); Gdl_flip(); // loading ..
    int n = currentNetworkCmds[ selected ].fn();
    if( n == -1 ) return selected ;
  };
}
