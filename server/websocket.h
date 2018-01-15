#ifndef __WEBSOCKET_H__
#define __WEBSOCKET_H__

#include "util.h"

#define ACCEPT_HEADER_V3 "HTTP/1.1 101 Switching Protocols\r\n"
#define ACCEPT_HEADER_V3_LEN 34
#define ACCEPT_UPGRADE "Upgrade: "
#define ACCEPT_UPGRADE_LEN 9
#define ACCEPT_CONNECTION "Connection: Upgrade\r\n"
#define ACCEPT_CONNECTION_LEN 21
#define ACCEPT_KEY "Sec-WebSocket-Accept: "
#define ACCEPT_KEY_LEN 22

void* wsHandle(void* client_arg);

// Where packet is gathered in packets from client
int communicate(ws_client* node, char* next, uint64_t next_len);

// Converts the unsigned 64 bit integer from host byte order to network byte order
uint64_t ntohl64(uint64_t value);

// encode the message as RFC6455
// returns 0 on success
int encodeMessage(ws_message* message);

// parses buffer
// returns 0 on success
int parseMessage(char* buffer, uint64_t buffer_length, ws_client* node);

// This function is suppose to get the remaining part of the message,
// if the message from the client is too big to be contained in the buffer.
// And we are dealing with the RFC6455 convention.
uint64_t getRemainingMessage(ws_client* node, uint64_t msg_length);

// used to take any message and figure out where to send callback
void callbackHandler(char* message);

#endif
