#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h> // open()
#include <pthread.h>

#include "server/server.h"
#include "modules/gpio.h"
#include "modules/wifi.h"

extern server_t* g_server;
static int status;

void socketCallback( int type, const char* value) {
  // To declare variables inside case you need to enable a scope with { }

  // printf("DEBUG - Data called back of %d : %s\n", type, value);
  switch(type) {

  case 0:
      break;

  case 1: {
    // LED lights as <gpioPin>:<status> as ints
    char *pEnd;
    int gpio, on;
    gpio = strtol(value, &pEnd, 10);
    on = strtol(pEnd+1, &pEnd, 10);
    GpioOutput(gpio, on);
    break;
  }
  case 2:
     break;

  default:
    printf("Not a valid type! [%d]\n", type);
    break;
  }
}

// Checks for GPIO input on pin 35
void* pollButton(void* notInUse) {
  struct pollfd pfd;
  int fd;
  char buf;
  GpioInput(35);
  
  fd = open("/sys/class/gpio/gpio35/value", O_RDONLY);
  if (fd < 0) {
    puts("ERROR: Could not open file:\n");
    return (void*) -1;
  }
  
  pfd.fd = fd;
  pfd.events = POLLPRI;
  
  // consume any prior interrupt
  lseek(fd, 0, SEEK_SET);
  read(fd, &buf, 1);

  
  while(1) {
    poll(&pfd, 1, -1);
    
    lseek(fd, 0, SEEK_SET);
    read(fd, &buf, 1);

    // buf is 0x30 or 0x31
    broadcastInt("5", buf & 0x1);
  }
}


int main ( int argc, char* argv[] ) {

  g_server = (server_t*)malloc(sizeof(server_t));
  g_server->port = 8000;
  g_server->onSocketMessage = socketCallback;

  startServer();

  int wMaxName = 48;
  int wMaxList = 16;
  char** wList;
  pthread_t buttonThread;

  // allocate for wifi list
  wList = (char**)malloc(sizeof(char*) * wMaxList);
  for (int i = 0; i < wMaxList; i++) {
    wList[i] = (char*)malloc(sizeof(char) * wMaxName);
  }

  // Kick off temperature thread
  int rc = pthread_create(&buttonThread, NULL, pollButton, NULL);
  if (rc) {
    printf("ERROR: Can't create button thread");
  }
  
  // main infinite loop
  while(1) {

    status = WifiScan(wList, wMaxList, wMaxName, 0x1);
    broadcastInt("3", 0);
    for (int i = 0; i < status; i++) {
      broadcastString("4", wList[i]);
    }

    usleep(5000000); // 5 sec
  }

  // CLEAN UP
  for (int i = 0; i < wMaxList; i++) {
    free(wList[i]);
  }
  free(wList);

}
