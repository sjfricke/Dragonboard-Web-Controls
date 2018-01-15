#include "api.h"

int callApiRoute(char** request_HTTP, char** response_HTTP, char* route, http_t* config)
{
  //  char* request_body;
  int status;
  
  // Get HTTP Verb Type
  if (strncmp(*request_HTTP, "GET", 3) == 0) {

    if (strncmp(route, "/key/", 5) == 0) {
      status = atoi(route + 5);
      // check if number or letter pressed
      if (status >= 48 && status <= 90) {
	config->onKeyPress((char)status);
	strcpy(*response_HTTP, "Key Received");
	return 1;
      } else {
	// TODO
	return 2;
      }
    }

  } else if (strncmp(*request_HTTP, "POST", 4) == 0) {

  } else {
    return printError("HTTP Verb not supported\n", -1);
  }


  // request_body = strstr(*http_request, "\n");
  
  return 0;
}
