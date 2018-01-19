#include "websocket.h"

extern server_t* g_server;

void* wsHandle(void* client_arg) {

  // detaches thread and sets up ws_client
  pthread_detach(pthread_self());
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

  ws_client* client = (ws_client*)client_arg;
  client->thread_id = pthread_self();

  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

  // first send handshake
  int memlen = 0;
  int length = 0;
  char* response = NULL;

  uint64_t next_len = 0;
  char next[BUFFER_SIZE];
  memset(next, '\0', BUFFER_SIZE);

  // get from adding values I did once... trust this please I guess
  length = ACCEPT_HEADER_V3_LEN
    + ACCEPT_UPGRADE_LEN +  client->header->upgrade_length
    + ACCEPT_CONNECTION_LEN
    + ACCEPT_KEY_LEN + client->header->accept_length
    + (2*3);

  response = getMemoryChar("", length);

  if (response == NULL) {
    printf("--SERVER-- ERROR: Allocating response ws\n");
  }

  memcpy(response + memlen, ACCEPT_HEADER_V3, ACCEPT_HEADER_V3_LEN);
  memlen += ACCEPT_HEADER_V3_LEN;

  memcpy(response + memlen, ACCEPT_UPGRADE, ACCEPT_UPGRADE_LEN);
  memlen += ACCEPT_UPGRADE_LEN;

  memcpy(response + memlen, client->header->upgrade, client->header->upgrade_length);
  memlen += client->header->upgrade_length;

  memcpy(response + memlen, "\r\n", 2);
  memlen += 2;

  memcpy(response + memlen, ACCEPT_CONNECTION, ACCEPT_CONNECTION_LEN);
  memlen += ACCEPT_CONNECTION_LEN;

  memcpy(response + memlen, ACCEPT_KEY, ACCEPT_KEY_LEN);
  memlen += ACCEPT_KEY_LEN;

  memcpy(response + memlen, client->header->accept, client->header->accept_length);
  memlen += client->header->accept_length;

  memcpy(response + memlen, "\r\n\r\n", 4);
  memlen += 4;

  if (memlen != length) {
    printf("--SERVER-- ERROR: We've fucked the counting up!\n");
  }

  // send handshake response
  send(client->socket_id, response, length, 0);

  if (NULL != response) {
    free(response);
  }

  // add new connection to list
  listAdd(g_server->list, client);
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

  printf("--SERVER-- Client %s was added like... a boss!\n", client->client_ip);

  // loop sending messages back and forth until close
  while (1) {
    if ( 0 != communicate(client, next, next_len) ) {
      break;
    }

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    listMulticast(g_server->list, client);
    callbackHandler(client->message->msg);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    if (client->message != NULL) {
      memset(next, '\0', BUFFER_SIZE);
      memcpy(next, client->message->next, client->message->next_len);
      next_len = client->message->next_len;
      messageFree(client->message);
      free(client->message);
      client->message = NULL;
    }
  }

  // clean up websocket
  printf("--SERVER-- Client %s decided it was to good and left\n", client->client_ip);

  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  listRemove(g_server->list, client);
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

  pthread_exit((void *) EXIT_SUCCESS);
}

int communicate(ws_client* node, char *next, uint64_t next_len) {
  int buffer_length = 0;
  uint64_t buf_len;
  char buffer[BUFFER_SIZE];
  int status;
  node->message = messageNew();

  if (node == NULL) {
    return printError("--SERVER-- ERROR: The client was not available anymore", -1);
  }

  if (node->header == NULL) {
    return printError("--SERVER-- ERROR: The header was not available anymore", -1);
  }

  // Receiving and decoding the message.
  do {
    memset(buffer, '\0', BUFFER_SIZE);
    memcpy(buffer, next, next_len);

    // If we end in this case, we have not got enough of the frame to
    // do something useful to it. Therefore, do yet another read operation
    if (next_len <= 6 || ((next[1] & 0x7f) == 126 && next_len <= 8) ||
	((next[1] & 0x7f) == 127 && next_len <= 14)) {

      if ((buffer_length = recv(node->socket_id, (buffer+next_len),
				(BUFFER_SIZE-next_len), 0)) <= 0) {
	return printError("--SERVER-- ERROR: didn't receive any message from client", -1);
      }
    }

    buf_len = (uint64_t)(buffer_length + next_len);

    // We need the opcode to conclude which type of message we received.
    if (node->message->opcode[0] == '\0') {
      memcpy(node->message->opcode, buffer, sizeof(node->message->opcode));
    }

    // Get the full message and remove the masking from it.
    status = parseMessage(buffer, buf_len, node);
    if (0 != status) {
      return status;
    }

    next_len = 0;
  } while( !(buffer[0] & 0x80) );

  // Checking which type of frame the client has sent.
  if (node->message->opcode[0] == '\x88' || node->message->opcode[0] == '\x08') {
    // CLOSE: client wants to close connection, so we do.
    return -2;

  } else if (node->message->opcode[0] == '\x8A' || node->message->opcode[0] == '\x0A') {
    return printError("--SERVER-- ERROR Pong arrived", -1);
  } else if (node->message->opcode[0] == '\x89' || node->message->opcode[0] == '\x09') {
    return printError("--SERVER-- ERROR Ping arrived", -1);
  } else if (node->message->opcode[0] == '\x02' || node->message->opcode[0] == '\x82') {
    return printError("--SERVER-- ERROR Binary data arrived", -1);
  } else if (node->message->opcode[0] == '\x01' || node->message->opcode[0] == '\x81') {

    // encode the message to make it ready to be send to all others
    status = encodeMessage(node->message);
    if ( 0 != status ) {
      return status;
    }

  } else {
    printf("--SERVER-- Something very strange happened, received opcode: 0x%x\n\n", node->message->opcode[0]);
    return -1;
  }

  return 0;
}

uint64_t ntohl64(uint64_t value) {
  static const int num = 42;

  // If these check is true, the system is using the little endian
  // convention. Else the system is using the big endian convention, which
  //  means that we do not have to represent our integers in another way.
  if (*(char *)&num == 42) {
    const uint32_t high = (uint32_t)(value >> 32);
    const uint32_t low = (uint32_t)(value & 0xFFFFFFFF);

    return (((uint64_t)(htonl(low))) << 32) | htonl(high);
  } else {
    return value;
  }
}

int encodeMessage(ws_message* message) {
  uint64_t length = message->len;
  uint16_t sz16;
  uint64_t sz64;

  if (message->len <= 125) {
    length += 2;
    message->enc = (char*) malloc(sizeof(char) * length);
    if (NULL == message->enc) {
      return printError("--SERVER-- ERROR: Couldn't allocate memory for message 001", -1);
    }

    message->enc[0] = '\x81';
    message->enc[1] = message->len;
    memcpy(message->enc + 2, message->msg, message->len);

  } else if (message->len <= 65535) {
    length += 4;
    message->enc = (char*) malloc(sizeof(char) * length);
    if (NULL == message->enc) {
      return printError("--SERVER-- ERROR: Couldn't allocate memory for message 002", -1);
    }

    message->enc[0] = '\x81';
    message->enc[1] = 126;
    sz16 = htons(message->len);
    memcpy(message->enc + 2, &sz16, sizeof(uint16_t));
    memcpy(message->enc + 4, message->msg, message->len);

  } else {
    length += 10;
    message->enc = (char*) malloc(sizeof(char) * length);
    if (message->enc == NULL) {
      return printError("--SERVER-- ERROR: Couldn't allocate memory for message 003", -1);
    }
    message->enc[0] = '\x81';
    message->enc[1] = 127;
    sz64 = ntohl64(message->len);
    memcpy(message->enc + 2, &sz64, sizeof(uint64_t));
    memcpy(message->enc + 10, message->msg, message->len);
  }
  message->enc_len = length;

  return 0;
}

int parseMessage(char* buffer, uint64_t buffer_length, ws_client* node) {
  ws_message* message = node->message;
  int length;
  int has_mask;
  int skip;
  int j;
  uint64_t message_length = message->len;
  uint64_t i;
  uint64_t remaining_length = 0;
  uint64_t buf_len;

  // Extracting information from frame
  has_mask = buffer[1] & 0x80 ? 1 : 0;
  length = buffer[1] & 0x7f;

  if (!has_mask) {
    printf("--SERVER-- ERROR Message didn't have masked data, received: 0x%x\n", buffer[1]);
    return -1;
  }

  /**
   * We need to handle the received frame differently according to which
   * length that the frame has set.
   *
   * length <= 125: We know that length is the actual length of the message,
   * 				  and that the maskin data must be placed 2 bytes further
   * 				  ahead.
   * length == 126: We know that the length is an unsigned 16 bit integer,
   * 				  which is placed at the 2 next bytes, and that the masking
   * 				  data must be further 2 bytes away.
   * length == 127: We know that the length is an unsigned 64 bit integer,
   * 				  which is placed at the 8 next bytes, and that the masking
   * 				  data must be further 2 bytes away.
   */
  if (length <= 125) {
    message->len += length;
    skip = 6;
    memcpy(&message->mask, buffer + 2, sizeof(message->mask));
  } else if (length == 126) {
    uint16_t sz16;
    memcpy(&sz16, buffer + 2, sizeof(uint16_t));

    message->len += ntohs(sz16);

    skip = 8;
    memcpy(&message->mask, buffer + 4, sizeof(message->mask));
  } else if (length == 127) {
    uint64_t sz64;
    memcpy(&sz64, buffer + 2, sizeof(uint64_t));

    message->len += ntohl64(sz64);

    skip = 14;
    memcpy(&message->mask, buffer + 10, sizeof(message->mask));
  } else {
    printf("--SERVER-- Obscure length received from client: %d\n\n", length);
    return -1;
  }

  // If the message length is greater that our MAXMESSAGE constant
  // we skip the message and close the connection
  if (message->len > MAX_MESSAGE_SIZE) {
    return printError("--SERVER-- Message received was bigger than MAX_MESSAGE_SIZE", -1);
  }

  // Allocating memory to hold the message sent from the client.
  // We can do this because we now know the actual length ofr the message.
  message->msg = (char*) malloc(sizeof(char) * (message->len + 1));
  if (message->msg == NULL) {
    return printError("--SERVER-- ERROR: Couldn't allocate memory 005\n", -1);
  }
  memset(message->msg, '\0', (message->len + 1));

  buf_len = (buffer_length-skip);

  // The message read from recv is larger than the message we are supposed to receive.
  // This means that we have received the first part of the next message as well
  if (buf_len > message->len) {
    uint64_t next_len = buf_len - message->len;
    message->next = (char*) malloc(next_len);
    if (message->next == NULL) {
      return printError("--SERVER-- ERROR: Couldn't allocate memory 006\n", -1);
    }
    memset(message->next, '\0', next_len);
    memcpy(message->next, buffer + (message->len+skip), next_len);
    message->next_len = next_len;
    buf_len = message->len;
  }

  memcpy(message->msg+message_length, buffer+skip, buf_len);

  message_length += buf_len;

  // We have not yet received the whole message, and must continue reading new data from the client
  if (message_length < message->len) {
    if ((remaining_length = getRemainingMessage(node, message_length)) == 0) {
      return printError("--SERVER-- ERROR: Closed Policy", -1);
    }
  }

  message_length += remaining_length;

  // If this is true, our receival of the message has gone wrong
  // and we have no other choice than closing the connection.
  if (message_length != message->len) {
    printf("--SERVER-- Message does not fit. Expected: %d but got %d\n\n",
	   (int) message->len, (int) message_length);
    return -1;
  }

  // If everything went well, we have to remove the masking from the data.
  for (i = 0, j = 0; i < message_length; i++, j++){
    message->msg[j] = message->msg[i] ^ message->mask[j % 4];
  }

  return 0;
}


uint64_t getRemainingMessage(ws_client* node, uint64_t msg_length) {
  int buffer_length = 0;
  uint64_t remaining_length = 0;
  uint64_t final_length = 0;
  char buffer[BUFFER_SIZE];
  ws_message* message = node->message;

  do {
    memset(buffer, '\0', BUFFER_SIZE);

    // Receive new chunk of the message.
    if ((buffer_length = recv(node->socket_id, buffer, BUFFER_SIZE, 0)) <= 0) {
      printf("--SERVER-- Didn't receive anything from remaining part of message. %d"
	     "\n\n", buffer_length);
      return 0;
    }

    // The overall length of the message received. Because the recv call
    // eventually will merge messages together we have to have a check
    // whether the overall length we received is greater than the expected
    // length of the message.
    final_length = (msg_length+remaining_length+buffer_length);

    // If the overall message is longer than the expected length of the
    // message, we know that this chunk most contain the last part of the
    // original message, and the first chunk of a new message.
    if ( final_length > message->len ) {
      uint64_t next_len = final_length-message->len;
      message->next = (char*) malloc(sizeof(char)*next_len);
      if (message->next == NULL) {
	return printError("--SERVER-- ERROR: Couldn't allocate memory for message 100\n", 0);
      }

      memset(message->next, '\0', next_len);
      memcpy(message->next, buffer + (buffer_length - next_len), next_len);
      message->next_len = next_len;
      buffer_length = buffer_length - next_len;
    }

    remaining_length += buffer_length;

    memcpy(message->msg + (msg_length+(remaining_length-buffer_length)), buffer,
	   buffer_length);
  } while( (msg_length + remaining_length) < message->len );

  return remaining_length;
}

// Expects all callbacks to be of form type:value
void callbackHandler(char* message) {

  char *err;

  // null pointer the first : ad send first part as number and rest as const char*
  char* value = strchr(message, ':');
  if (value == NULL) { return; }
  memset(value, '\0', 1);

  long int type = strtol(message, &err, 10);

  g_server->onSocketMessage(type, (const char*)value+1);
}
