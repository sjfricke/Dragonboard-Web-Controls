#ifndef __HTTP_H__
#define __HTTP_H__

#include "util.h"
//#include "api.h"

void* httpHandle(http_client* http_config);

// Takes files and sets it right align with the memeory at
// return_body of size length
// Returns the length of content length or -1 if error
int getFileContent(char*  relative_path,
		   char** return_body,
		   int    length);

#endif
