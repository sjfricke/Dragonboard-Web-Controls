CC = gcc
CFLAGS = -Wall -Werror -std=gnu11
OBJ_SERVER = server/util.o server/server.o server/websocket.o server/http.o server/sha1.o server/base64.o
OBJ_MOD = modules/i2c.o modules/gpio.o
SHARED = -lpthread

main: modules server
	$(CC) $(OBJ_SERVER) $(OBJ_MOD) main.c $(CFLAGS) $(SHARED) -o DragonWeb

test: modules server
	$(CC) $(OBJ_SERVER) $(OBJ_MOD) test.c $(CFLAGS) $(SHARED) -o Test

modules: $(OBJ_MOD)

modules/%.o: modules/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

server: $(OBJ_SERVER)

server/%.o: server/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm modules/*.o
	rm server/*.o
	rm DragonWeb
	rm Test
