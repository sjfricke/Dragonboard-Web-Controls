#include "server.h"

// TODO globa?
pthread_t server_thread;
pthread_t ws_thread;

static int status; // used to check status of c functions return values

// Takes header and adds a valid SHA-1 key
static void getSHA(request_header* header) {

  SHA1Context sha;
  char* magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  int magic_len = 36;
  int length = magic_len + strlen(header->ws_key);
  uint32_t number;
  char key[length];
  char sha1Key[20];
  char* acceptKey = NULL;
  int i;

  memset(key, '\0', length);
  memset(sha1Key, '\0', 20);

  memcpy(key, header->ws_key, (length-magic_len));
  memcpy(key+(length-magic_len), magic, magic_len);

  SHA1Reset(&sha);
  SHA1Input(&sha, (const unsigned char*) key, length);

  if ( !SHA1Result(&sha) ) {
    printf("ERROR: doing SHA hash key from %s\n", key);
    return;
  }

  for(i = 0; i < 5; i++) {
    number = ntohl(sha.Message_Digest[i]);
    memcpy(sha1Key+(4*i), (unsigned char *) &number, 4);
  }

  if (base64_encode_alloc((const char *) sha1Key, 20, &acceptKey) == 0) {
    printf("--SERVER-- ERROR: The input length was greater than the output length\n");
    return;
  }

  if (acceptKey == NULL) {
    printf("--SERVER-- ERROR: Couldn't allocate memory.\n");
    return;
  }

  header->accept = acceptKey;
  header->accept_length = strlen(header->accept);
}

// sends a formated string to all sockets
static void broadcast(char* broadcast_string)
{
  ws_message *message = messageNew();
  message->len = strlen(broadcast_string);

  char *temp = malloc( sizeof(char)*(message->len+1) );
  if (temp == NULL) {
    raise(SIGINT);
    return;
  }

  memset(temp, '\0', (message->len+1));
  memcpy(temp, broadcast_string, message->len);
  message->msg = temp;
  temp = NULL;

  if ( (status = encodeMessage(message)) != 0) {
    messageFree(message);
    free(message);
    raise(SIGINT);
    return;
  }

  listMulticastAll(g_server->list, message);
  messageFree(message);
  free(message);
}

// takes request and parses header to struct
// returns header if valid, NULL if error
static request_header* parseHeader(char** request) {

  // Defaults are set which we used to infer it wasn't found in header
  request_header* header = headerNew();
  if (NULL == header) {
    return printErrorNull("--SERVER-- Failed to allocate header in parsing");
  }

  char* token = strtok(*request, "\r\n");
  char* route;
  int route_length = 0;

  if (token != NULL) {

    // first check for Verb
    if ( 0 == strncasecmp("GET /", token, 5)) {
      header->verb = GET;
    } else if ( 0 == strncasecmp("POST /", token, 6)) {
      header->verb = POST;
    } else {
      return printErrorNull("--SERVER-- Not Get or Post");
    }

    route = strstr(*request, "/");
    // finds where route path ends
    while (route[route_length] != ' ') {
      route_length++;
    }

    // allocates header and makes sure to add \0 to end string
    header->route = (char*)malloc(route_length+1);
    memcpy(header->route, route, route_length);
    header->route[route_length] = '\0';

    // time to loop through header lines
    while ( token != NULL ) {

      if (0 == strncasecmp("Sec-WebSocket-Version: ", token, 23)) {
	header->ws_version = strtol(token+23, (char**) NULL, 10);
      } else if (0 == strncasecmp("Sec-WebSocket-Key: ", token, 19)) {
	header->ws_key = token + 19;
      } else if (0 == strncasecmp("Upgrade: ", token, 9)) {
	header->upgrade = token + 9;
	header->upgrade_length = strlen(header->upgrade);
      }

      token = strtok(NULL, "\r\n");
    }

    // time to validate and determine what we were requested
    if ( 0 == header->ws_version ) {
      // HTTP
      header->type = HTTP;
      return header;

    } else if ( 13 == header->ws_version ) {

      if ( 0 == strncasecmp(header->upgrade, "websocket", 9) &&
	   (NULL != header->upgrade) && (NULL != header->ws_key)) {
	// websocket RFC6455
	header->type = WEBSOCKET;

	// Need to create SHA1 key
	getSHA(header);

	return header;

      }	else {
	return printErrorNull("--SERVER-- Need Socket upgrade and key in header");
      }
    } else {
      return printErrorNull("--SERVER-- Only RFC6455 Websockets supported");
    }
  } else {
    return printErrorNull("--SERVER-- Parse Header Error!");
  }
}

// Creates a new thread to run the Daemon server
static void* serverDaemon() {

  //--------------------------------//
  //         Variable Setup         //
  //--------------------------------//

  int port = g_server->port;
  int on = 1;

  http_client* http_config = NULL;
  char* request_HTTP = malloc(MAX_REQUEST_SIZE);
  int   request_size;

  char* temp;
  char* client_ip;

  //--------------------------------//
  //       Configure TCP Socket     //
  //--------------------------------//

  struct sockaddr_in client;    // socket info about the machine connecting to us
  struct sockaddr_in server;    // socket info about our server
  int socket_fp;                // socket used to listen for incoming connections
  int socket_con;               // used to hold status of connect to socket
  socklen_t socket_size = sizeof(struct sockaddr_in);

  memset(&server, 0, sizeof(server));          // zero the struct before filling the fields
  server.sin_family = AF_INET;                 // set to use Internet address family
  server.sin_addr.s_addr = htonl(INADDR_ANY);  // sets our local IP address
  server.sin_port = htons(port);               // sets the server port number

  // creates the socket
  // AF_INET refers to the Internet Domain
  // SOCK_STREAM sets a stream to send data
  // 0 will have the OS pick TCP for SOCK_STREAM
  socket_fp = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fp < 0) {
    printf("--SERVER-- ERROR: Trying to Opening socket\n");
    pthread_exit(NULL);
  }

  // This prevents the TIME_WAIT socket error on reloading
  status = setsockopt(socket_fp, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  if (status < 0) {
    printf("--SERVER-- ERROR: setting SOL_SOCKET\n");
    pthread_exit(NULL);
  }

  // bind server information with server file poitner
  status = bind(socket_fp, (struct sockaddr *)&server, sizeof(struct sockaddr));

  // checks for TIME_WAIT socket
  // when daemon is closed there is a delay to make sure all TCP data is propagated
  if (status < 0) {
    printf("--SERVER-- ERROR opening socket: %d , possible TIME_WAIT\n", status);
    printf("\tUSE: netstat -ant|grep %d to find out\n", port);
    pthread_exit(NULL);
  }

  // start listening, allowing a queue of up to 10 pending connection
  listen(socket_fp, 10);
  printf("--SERVER-- Ready on port %d!\n\n", port);

  //--------------------------------//
  //         Server Polling         //
  //--------------------------------//

  //blocking for response
  socket_con = accept(socket_fp, (struct sockaddr *)&client, &socket_size);

  while(1) {

    // get IP
    temp = (char*) inet_ntoa(client.sin_addr); // gets address
    client_ip = (char*) malloc(sizeof(char)*(strlen(temp)+1));
    if (NULL == client_ip) {
      printf("--SERVER-- ERROR: Allocating client_ip\n");
      exit(1); // TODO
    }
    // extra \0 to have it be a string with ending char
    memset(client_ip, '\0', strlen(temp)+1);
    memcpy(client_ip, temp, strlen(temp));

    // get request
    request_size = recv(socket_con, request_HTTP, MAX_REQUEST_SIZE, 0);
    if (request_size <= 0) {
      printf("--SERVER-- Request Size was %d\n", request_size);
    }

    request_header* header = parseHeader(&request_HTTP);
    if (NULL == header) {
      printf("--SERVER-- Couldn't allocate header!\n");
    }

    if ( HTTP == header->type ) {
      // http_client created only once, can update if not first time
      if (NULL == http_config) {
	http_config = httpClientNew(socket_con, client_ip);
      } else {
	http_config->socket_id = socket_con;
	http_config->client_ip = client_ip;
      }
      // we clear the request_header on each send() though
      http_config->header = header;

      httpHandle(http_config);

    } else if ( WEBSOCKET == header->type ) {

      // create ws_client to pass across the info
      ws_client* ws_node = wsClientNew(socket_con, client_ip);
      ws_node->header = header;

      // opens new thread to keep communication with socket
      status = pthread_create(&ws_thread,
			      NULL,
			      wsHandle,
			      (void *)ws_node);

      if (status < 0) {
	printf("--SERVER-- ERROR: Are you feeling it now Mr Krabs?\n");
      }

      pthread_detach(ws_thread);
    }

    socket_con = accept(socket_fp, (struct sockaddr *)&client, &socket_size);
  }

  pthread_exit(NULL);
}


void startServer() {

  if (g_server->port <= 1024 || g_server->port >= 65536) {
    printf("--SERVER-- ERROR: Port must be between 1024 and 65536\n");
    exit(1);
  }

  // creates new WS list
  g_server->list = listNew();

  status = pthread_create(&server_thread, NULL, serverDaemon, NULL);

  if (status < 0) {
    printf("--SERVER-- ERROR: Server didn't start up correctly\n");
    exit(1);
  }

}

void broadcastInt(char* type, int value)
{
  char broadcast_string[21 + strlen(type) + 9];
  sprintf(broadcast_string, "{\"type\":\"%s\",\"value\":%d}", type, value);
  broadcast(broadcast_string);
}

void broadcastDouble(char* type, double value)
{
  char broadcast_string[21 + strlen(type) + 9];
  sprintf(broadcast_string, "{\"type\":\"%s\",\"value\":%9.3f}", type, value);
  broadcast(broadcast_string);
}

void broadcastString(char* type, char* value)
{
  char broadcast_string[23 + strlen(type) + strlen(value)];
  sprintf(broadcast_string, "{\"type\":\"%s\",\"value\":\"%s\"}", type, value);
  broadcast(broadcast_string);
}
