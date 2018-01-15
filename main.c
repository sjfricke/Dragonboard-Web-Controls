#include "server/server.h" 

#include <stdio.h>
#include <unistd.h>

extern server_t* g_server;

static char command[256];
uint16_t speech_marker = 0;

void webDataCallback( int type, char* value) {
  FILE *fp; // c99 pre declaring
  
  switch(type) {

  case 0:
    browser_connected = TRUE;
    break;

  case 1:
    break;

  case 2:
     break;
    
  default:
    printf("Not a valid type! [%d]\n", type);
    break;
  }
}

int main ( int argc, char* argv[] ) {

  char command[256];

  g_server = (server_t*)malloc(sizeof(server_t));
  g_server->port = 6419;
  g_server->onData = webDataCallback;

  startServer();

  //   // Kick off temperature thread
  // int rc = pthread_create(&tempThread, NULL, pollTemperature, NULL);
  // if (rc) {
  //   printf("ERROR: Can't create temperature thread");
  // }

  while(1) {

    usleep(10000); // 10ms
    
  }
}
