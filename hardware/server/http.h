#ifndef __HTTP_H__
#define __HTTP_H__

#include "util.h"

#define WEBSITE_FOLDER "../website"

#define BAD_REQ "400 BAD REQUEST"
#define GOOD_REQ "200 OK"

typedef struct http_client {
  int socket_id;
  char* client_ip;
  char* response_HTTP;
  char* response_header;
  char* timestamp;
  char* content_type;
  request_header* header;
} http_client;


void* httpHandle(http_client* http_config);

// Takes files and sets it right align with the memeory at
// return_body of size length
// Returns the length of content length or -1 if error
int getFileContent(char*  relative_path,
		   char** return_body,
		   char*  content_type,
		   int    length);

// used to create and destroy http objects
http_client* httpClientNew(int socket_con, char* address);
void httpClientFree(http_client* client);

void getMime(char* ext, char* mime);

#endif
