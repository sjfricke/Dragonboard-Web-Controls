#ifndef __SERVER_UTIL_H__
#define __SERVER_UTIL_H__

#include <sys/types.h>  // socket, setsockopt, accept, send, recv
#include <sys/socket.h> // socket, setsockopt, inet_ntoa, accept
#include <netinet/in.h> // sockaddr_in, inet_ntoa
#include <arpa/inet.h> 	// htonl, htons, inet_ntoa

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include "sha1.h"
#include "base64.h"

#define PRINT_ERRORS 1

#define BUFFER_SIZE 8192          // 8KB
#define MAX_MESSAGE_SIZE 1048576  // 1MB

// Size used when allocating memory
#define MAX_RESPONSE_SIZE 5000000 // 5MB
#define MAX_HEADER_SIZE   20000   // 20kb
#define MAX_REQUEST_SIZE  500000  // 500KB

// function callbacks of different arg types
typedef void (*callbackStr)(char*);
typedef void (*callbackChar)(char);
typedef void (*callbackInt)(int);
typedef void (*callbackDouble)(double);
typedef void (*callbackIntStr)(int, char*);
typedef void (*callbackIntConstStr)(int, const char*);

typedef enum {
  GET,
  POST,
  UKKNOWN
} VERB;

typedef enum {
  HTTP,
  WEBSOCKET,
  API,
  UNKNOWN
} request_type;

typedef struct {
  char opcode[1];
  char mask[4];
  uint64_t len;
  uint64_t enc_len;
  uint64_t next_len;
  char *msg;
  char *next;
  char *enc;
} ws_message;

// Info found in a WS and HTTP header
typedef struct {
  VERB verb;
  request_type type;
  int ws_version;
  char* route;
  char* ws_key;
  char* accept;
  char* upgrade;
  char* client_ip;
  int accept_length;
  int upgrade_length;
} request_header;

// Info for each ws thread created
typedef struct ws_client_n {
  int socket_id;
  char* client_ip;
  pthread_t thread_id;
  request_header* header;
  ws_message* message;
  struct ws_client_n* next;
} ws_client;

// We need a list to hold all the current WS clients
typedef struct {
  int len;
  ws_client *first;
  ws_client *last;
  pthread_mutex_t lock;
} ws_list;

// Configuration info sent from server
typedef struct server_t {
  int                  port;
  ws_list*             list;
  callbackIntConstStr  onSocketMessage;
} server_t;

//////////////////////////////////////////
////////////    Functions    /////////////
//////////////////////////////////////////

// makes error 1 liners
int printError(char* message,
	       int   return_val);

// makes error 1 line with returning NULL
void* printErrorNull(char* message);

// prints before exiting program
void printFatal(char* message, int id);

// Takes a string and length and sets it to RFC 1123 Date Format for HTTP Response
void getTime(char** timestamp,
	     int    length);

// List functions.
ws_list* listNew(void);
void listFree(ws_list* list);
void listAdd(ws_list* list, ws_client* node);
void listRemove(ws_list* list, ws_client* remove);
void listRemoveAll(ws_list* list);
void listPrint(ws_list* list);
void listMulticast(ws_list* list, ws_client* node);
void listMulticastAll(ws_list* list, ws_message* message);

// Websocket functions.
void wsCloseframe(ws_client* client);
void wsSend(ws_client* client, ws_message* message);

// Memory handling functions
char* getMemoryChar(char* token, int length);

// New structures.
ws_client* wsClientNew(int socket_con, char* address);
request_header* headerNew();
ws_message* messageNew();

// Free structures
void headerFree(request_header* header);
void messageFree(ws_message* message);
void wsClientFree(ws_client* client);

#endif
