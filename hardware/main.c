#include <stdio.h>
#include <unistd.h>
#include "server/server.h"
#include "modules/gpio.h"

extern server_t* g_server;

static char command[256];
uint16_t speech_marker = 0;

void socketCallback( int type, const char* value) {
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
  g_server->onSocketMessage = socketCallback;

  startServer();


  int button = GpioInputPin(33);
  int led = GpioOutputPin(34, 0);
  
  temp_file = fopen(sys_temp, "r");
  while(1) {

    // remember buttons are pull-down resistors
    int button_status = GpioGetValue(button);
    if (button_status == 0) {
      broadcastInt("5", 1);
    } else {
      broadcastInt("5", 0);
    }
    //fscanf(temp_file, "%lf", &cur_temp);
    // cur_temp /= 1000;
   //printf("Temp: %lf degrees\n", cur_temp);
    // rewind(temp_file); // need to rewind file pointer
    usleep(1000000); // 1 sec
  }
}
