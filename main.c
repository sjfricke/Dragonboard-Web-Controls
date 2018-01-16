#include "server/server.h" 

#include <stdio.h>
#include <unistd.h>

extern server_t* g_server;

static char command[256];
uint16_t speech_marker = 0;

void webDataCallback( int type, char* value) {
  FILE *fp; // c99 pre declaring
  printf("Data called back of %d : %s\n", type, value); 
  switch(type) {

  case 0:
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

  double cur_temp;
  FILE* temp_file;
  char* sys_temp = "/sys/class/thermal/thermal_zone0/temp";
  
  g_server = (server_t*)malloc(sizeof(server_t));
  g_server->port = 8000;
  g_server->onData = webDataCallback;

  startServer();

  temp_file = fopen(sys_temp, "r");
  while(1) {
    fscanf(temp_file, "%lf", &cur_temp);
    cur_temp /= 1000;
    printf("Temp: %lf degrees\n", cur_temp);
    rewind(temp_file); // need to rewind file pointer
    usleep(1000000); // 1 sec
  }
}
