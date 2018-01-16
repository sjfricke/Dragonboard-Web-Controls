// apt install libiw-dev
// add -liw in compile line
// Docs: http://docs.ros.org/jade/api/heatmap/html/iwlib_8h.html

#include <stdio.h>
#include <time.h>
#include <iwlib.h>

int main(void) {
  wireless_scan_head head;
  wireless_scan *result;
  iwrange range;
  int sock;
  char buffer[256];

  /* Open socket to kernel */
  sock = iw_sockets_open();

  /* Get some metadata to use for scanning */
  if (iw_get_range_info(sock, "wlan0", &range) < 0) {
    printf("Error during iw_get_range_info. Aborting.\n");
    exit(2);
  }

  /* Perform the scan */
  if (iw_scan(sock, "wlan0", range.we_version_compiled, &head) < 0) {
    printf("Error during iw_scan. Aborting.\n");
    exit(2);
  }

  /* Traverse the results */
  result = head.result;
  while (NULL != result) {
    printf("%s\n", result->b.essid);
    

    iw_print_stats(buffer,
		   sizeof(buffer),
		   &result->stats.qual,
		   &range,
		   1);
    printf("stats: %s\n", buffer);
    
    result = result->next;
  }

  puts("------------------\n");
  exit(0);
}
