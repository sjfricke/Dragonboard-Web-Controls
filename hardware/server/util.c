#include "util.h"

int printError(char* message, int return_val) {
  if (1 == PRINT_ERRORS) {
    printf("%s\n", message);
  }
  return return_val;
}

void* printErrorNull(char* message) {
  if (1 == PRINT_ERRORS) {
    printf("%s\n", message);
  }
  return NULL;
}

void printFatal(char* message, int id) {
  printf("\n*************************************\n");
  printf("Fatel Error %d:\n%s\n", id, message);
  printf("*************************************\n");
  exit(id);
}

void getTime(char** timestamp, int length) {

  time_t raw_time;
  struct tm *info;
  time( &raw_time );

  info = localtime( &raw_time );

  strftime(*timestamp, length, "%a, %d %b %Y %H:%M:%S %Z", info);  
}


// Creates a new list structure.
// return Empty list
ws_list* listNew (void) {
  ws_list* list = (ws_list*) malloc(sizeof(ws_list));
	
  if (NULL != list) {
    list->len = 0;
    list->first = list->last = NULL;
    pthread_mutex_init(&list->lock, NULL);	
  } else {
    printFatal("Can't allocated ws_list*", 20);
  }
  
  return list;
}

// Frees the list structure, including all its nodes.
void listFree (ws_list* list) {
  ws_client *node, *previous;
  pthread_mutex_lock(&list->lock);
  node = list->first;
  
  while (NULL != node) {
    previous = node->next;
    wsCloseframe(node);
    // sys/socket shutdown call
    // DRWR disables further send and receives
    shutdown(node->socket_id, SHUT_RDWR);
    
    wsClientFree(node);    
    close(node->socket_id);
    free(node);
    node = previous;
  }

  list->first = list->last = NULL;
  pthread_mutex_unlock(&list->lock);	
  pthread_mutex_destroy(&list->lock);
  free(list);
}

// Adds a node to the list
void listAdd (ws_list* list, ws_client* node) {
  pthread_mutex_lock(&list->lock);
  
  if (NULL != list->first) {
    list->last = list->last->next = node;	
  } else {
    list->first = list->last = node;	
  }
  
  list->len++;
  pthread_mutex_unlock(&list->lock);
}

// Removes a node from the list, and sends closing frame to the client
void listRemove (ws_list* list, ws_client* remove) {
  ws_client *node, *previous;
  pthread_mutex_lock(&list->lock);
  node = list->first;
  
  if (NULL == node || NULL == remove) {
    pthread_mutex_unlock(&list->lock);
    return;
  }
  
  do {
    if (node == remove) {
      if (node == list->first) {
	list->first = node->next;
      } else {
	previous->next = node->next;
      }
      
      if (node == list->last) {
	list->last = previous;
      }

      wsCloseframe(node);
      shutdown(node->socket_id, SHUT_RDWR);

      wsClientFree(node);
      close(node->socket_id);
      free(node);

      list->len--;
      break;
    }
		
    previous = node;
    node = node->next;
  } while (NULL != node); 

  if (0 == list->len) {
    list->first = list->last = NULL;
  } else if (1 == list->len) {
    list->last = list->first;
  }

  pthread_mutex_unlock(&list->lock);
}

// Function that will send closeframe to each client in the list.
void listRemoveAll (ws_list* list) {
  ws_client* node;

  pthread_mutex_lock(&list->lock);
  node = list->first;

  if (NULL == node) {
    pthread_mutex_unlock(&list->lock);
    return;
  }

  do {
    wsCloseframe(node);
    node = node->next;
  } while (NULL != node); 

  pthread_mutex_unlock(&list->lock);
}

// Prints out information about each node contained in the list.
// mainly for debugging
void listPrint(ws_list* list) {
  ws_client* node;
  pthread_mutex_lock(&list->lock);
  node = list->first;

  if (NULL == node) {
    printf("No websocket clients are online\n");
    fflush(stdout);
    pthread_mutex_unlock(&list->lock);
    return;
  }

  do {
    printf("Socket Id: \t\t%d\n"
	   "pthread Id: \t\t%lu\n",
	   node->socket_id, (unsigned long)node->thread_id);
    fflush(stdout);
    node = node->next;
  } while (NULL != node);
  pthread_mutex_unlock(&list->lock);
}

// Multicasts message to all client in the list except sender
void listMulticast(ws_list* list, ws_client* node) {
  ws_client* previous;
  pthread_mutex_lock(&list->lock);
  previous = list->first;

  if (NULL == previous) {
    pthread_mutex_unlock(&list->lock);
    return;
  }

  do {
    if (previous != node) {
      wsSend(previous, node->message);
    }
    previous = previous->next;
  } while (NULL != previous);
  pthread_mutex_unlock(&list->lock);
}

// Multicasts message to all client in the list.
void listMulticastAll(ws_list* list, ws_message* message) {
  ws_client* previous;
  pthread_mutex_lock(&list->lock);
  previous = list->first;

  if (NULL == previous) {
    pthread_mutex_unlock(&list->lock);
    return;
  }

  do {
    wsSend(previous, message);
    previous = previous->next;
  } while (NULL != previous);
  pthread_mutex_unlock(&list->lock);
}

// Functions which creates the closeframe.
void wsCloseframe(ws_client* client) {
  char frame[2];
  
  frame[0] = '\x88';
  frame[1] = '\x00';

  send(client->socket_id, frame, 2, 0);
  pthread_cancel(client->thread_id);
}

// Function which do the actual sending of messages.
void wsSend(ws_client* client, ws_message* message) {
  send(client->socket_id, message->enc, message->enc_len, 0);
}

char* getMemoryChar(char* token, int length) {
  char* temp = (char*) malloc(length);

  if (NULL == temp) {
    return NULL;
  }

  memset(temp, '\0', length);
  memcpy(temp, token, length);
  return temp;
}

// Creates a new we client object
ws_client* wsClientNew (int socket_con, char* address) {
  ws_client* node = (ws_client*) malloc(sizeof(ws_client));

  if (NULL != node) {
    node->socket_id = socket_con;
    node->client_ip = address;
    node->thread_id = 0;
    node->header = NULL;
    node->next = NULL;
    node->message = NULL;
  }

  return node;
}

// Creates a new http client object
http_client* httpClientNew (int socket_con, char* address) {
  http_client* node = (http_client*) malloc(sizeof(http_client));

  if (NULL != node) {
    node->socket_id = socket_con;
    node->client_ip = address;
    node->header = NULL;
    node->response_HTTP = (char*)malloc(MAX_RESPONSE_SIZE + 2); // +2 for \r\n
    node->response_header = (char*)malloc(MAX_HEADER_SIZE); // TODO reset limit?
    node->timestamp = (char*)malloc(sizeof(char)*256);
  }

  return node;
}

// Creates a new header structure.
request_header* headerNew() {
  request_header* header = (request_header*) malloc(sizeof(request_header));

  if (NULL != header) {
    header->verb = UNKNOWN;
    header->type = UNKNOWN;
    header->ws_version = 0;
    header->route = NULL;
    header->ws_key = NULL;
    header->accept = NULL;
    header->upgrade = NULL;
    header->client_ip = NULL;
    header->accept_length = 0;
    header->upgrade_length = 0;
  }

  return header;
}

// Creates a new message structure.
ws_message* messageNew() {
  ws_message* message = (ws_message*) malloc(sizeof(ws_message));

  if (NULL != message) {
    memset(message->opcode, '\0', 1);
    memset(message->mask, '\0', 4);
    message->len = 0; 
    message->enc_len = 0;
    message->next_len = 0;
    message->msg = NULL;
    message->next = NULL;
    message->enc = NULL;
  }

  return message;	
}

// Frees all allocations in the header structure.
void headerFree(request_header* header) {

  if (NULL != header->accept) {
    free(header->accept);
    header->accept = NULL;
  }
  
  if (NULL != header->route) {
    free(header->route);
    header->route = NULL;
  }
  
  /* if (NULL != header->ws_key) {
    free(header->ws_key);
    header->ws_key = NULL;
  }

  if (NULL != header->upgrade) {
    free(header->upgrade);
    header->upgrade = NULL;
  }

  if (NULL != header->client_ip) {
    free(header->client_ip);
    header->client_ip = NULL;
  }*/
}

// Frees all allocations in the message structure.
void messageFree(ws_message* message) {
  if (NULL != message->msg) {
    free(message->msg);
    message->msg = NULL;
  }
	
  if (NULL != message->enc) {
    free(message->enc);
    message->enc = NULL;
  }

  if (NULL != message->next) {
    free(message->next);
    message->next = NULL;
  }

}

// Frees all allocations in the node, including the header and message 
void wsClientFree(ws_client* client) {

  if (NULL != client->client_ip) {
    free(client->client_ip);
    client->client_ip = NULL;
  }
  
  if (NULL != client->header) {
    headerFree(client->header);
    free(client->header);
    client->header = NULL;
  }

  if (NULL != client->message) {
    messageFree(client->message);
    free(client->message);
    client->message = NULL;
  }
}

// Frees all allocations in the node, including the header and message 
void httpClientFree(http_client* client) {

  if (NULL != client->client_ip) {
    free(client->client_ip);
    client->client_ip = NULL;
  }
  
  if (NULL != client->header) {
    headerFree(client->header);
    free(client->header);
    client->header = NULL;
  }

  if (NULL != client->response_HTTP) {
    free(client->response_HTTP);
    client->response_HTTP = NULL;
  }
  
  if (NULL != client->response_header) {
    free(client->response_header);
    client->response_header = NULL;
  }
  
  if (NULL != client->timestamp) {
    free(client->timestamp);
    client->timestamp = NULL;
  }
}
