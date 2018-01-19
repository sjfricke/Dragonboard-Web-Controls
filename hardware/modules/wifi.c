#include "wifi.h"

int WifiScan(char** list, int maxListLen, int maxNameLen) {

  wireless_scan_head head;
  wireless_scan *result;
  iwrange range;
  int sock;
  int resCount = 0;
  int scanCount = 0;
  char buffer[256];
  /* Open socket to kernel */
  sock = iw_sockets_open();

  /* Get some metadata to use for scanning */
  if (iw_get_range_info(sock, "wlan0", &range) < 0) {
    printf("--WIFI-- Error during iw_get_range_info. Aborting.\n");
    return -1;
  }

  /* Perform the scan */
  if (iw_scan(sock, "wlan0", range.we_version_compiled, &head) < 0) {
    printf("--WIFI-- Error during iw_scan. Aborting.\n");
    return -1;
  }
  
  /* Traverse the results */
  result = head.result;
  while (NULL != result && resCount < maxListLen) {
    
    strncpy(list[resCount], result->b.essid, maxNameLen);

    iw_print_stats(buffer,
		   sizeof(buffer),
		   &result->stats.qual,
		   &range,
		   1);
    printf("stats: %s\n", buffer);
    
    result = result->next;
    scanCount++;
    resCount++;
  }

  return scanCount;
}
