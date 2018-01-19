#ifndef __SERVER_SERVER_H__
#define __SERVER_SERVER_H__

#include "util.h"
#include "http.h"
#include "websocket.h"

// Single global server instances
server_t* g_server;
// Global threads
pthread_t server_thread;
pthread_t ws_thread;

////////////////////////////////////
/////////// Server Methods /////////
////////////////////////////////////

// sets default server_t and returns it back
server_t* setupSever(void);

// Call to spin up an HTTP sever
void startServer();

// Used to broadcast to all websockets a number as a double or int
void broadcastDouble(char* type, double value);
void broadcastInt(char* type, int value);

// overload method to broadcast string
void broadcastString(char* type, char* value);

#endif
