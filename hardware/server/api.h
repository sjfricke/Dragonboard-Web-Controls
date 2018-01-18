#ifndef __API_H__
#define __API_H__

#include "util.h"

// takes route and finds where to route it
// returns positive if success
//         0 if no route found
//         negative if error
int callApiRoute(char**    request_HTTP,
		 char**    response_HTTP,
		 char*     rotue,
		 http_t*   config);

#endif
