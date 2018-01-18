#ifndef __SERVER_SERVER_H__
#define __SERVER_SERVER_H__

#include "util.h"
#include "http.h"
#include "websocket.h"

// Single global server instances
server_t* g_server;

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


//////////////////////////////////////
/////////  Internal Helpers //////////
//////////////////////////////////////
// TODO make these static

// Creates a new thread to run the Daemon server
void* serverDaemon();

// sends a formated string to all sockets
void broadcast(char* broadcast_string);

// takes request and parses header to struct
// returns header if valid, NULL if error
request_header* parseHeader(char** request_all);

// Takes header and adds a valid SHA-1 key
void getSHA(request_header* header);

#endif
