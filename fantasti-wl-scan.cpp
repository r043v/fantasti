
#include <iwlib.h>
#include <stdlib.h>

#define u32 unsigned int

int main(int argc, char const *argv[]) {
  char * dev = (char*)argv[1];

	iwrange range;
	int sock = iw_sockets_open();

	/* Get some metadata to use for scanning */
	if (iw_get_range_info(sock, dev, &range) < 0) return 1;

	wireless_scan_head head;
	wireless_scan *result;
  u32 scan = 0, test = 0;

  int ret = iw_scan(sock, dev, range.we_version_compiled, &head);

  while( ret < 0 && test < 5 ){
    sleep( 1 );
    test++;
    ret = iw_scan(sock, dev, range.we_version_compiled, &head);
  };

  if( ret < 0 ){
    iw_sockets_close(sock);
    return 1;
  }

//  char * labels = (char*)malloc(1024), *l = labels;
//  char * label[64];
//  u32 nb = 0;

  result = head.result;
  while( result != NULL ){
//    label[ nb++ ] = l;
//    char * p = result->b.essid;
//    while( *l++ = *p++ );
    printf("%s\n", result->b.essid);
    wireless_scan *next = result->next;
    free( result );
    result = next;
  };

  iw_sockets_close(sock);

//  label[ nb ] = NULL;

  return 0;
}
