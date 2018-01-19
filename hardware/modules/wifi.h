// apt install libiw-dev
// add -liw in compile line
// Docs: http://docs.ros.org/jade/api/heatmap/html/iwlib_8h.html

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <iwlib.h>

// Scans Wifi and copies to list passed in
// caller needs to allocate and set the max length or both
// the number of names in list and max length per name
// returns number of addres found, neg if error
//
// Options
// 1: Will list dBm with name
int WifiScan(char** list, int maxListLen, int maxNameLen, int options);


