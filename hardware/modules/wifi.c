#include "wifi.h"

int wifiScan(char** list, int max_list_len, int max_name_len, int options) {

  wireless_scan_head head;
  wireless_scan *result;
  iwrange range;
  int sock;
  int res_count = 0;
  int scan_count = 0;
  double rcpilevel;
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
  while (NULL != result && res_count < max_list_len) {

    if ((options & 0x1) != 0) {
      rcpilevel = (result->stats.qual.level / 2.0) - 110.0;
      sprintf(buffer, "%s [ %g dBm ]", result->b.essid, rcpilevel);

      strncpy(list[res_count], buffer, max_name_len);
    } else {
      strncpy(list[res_count], result->b.essid, max_name_len);
    }

    result = result->next;
    scan_count++;
    res_count++;
  }

  return scan_count;
}
