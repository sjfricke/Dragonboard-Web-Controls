CC = gcc
CFLAGS = -Wall -Werror -c
OBJ_SERVER = server/util.o server/server.o server/websocket.o server/http.o server/sha1.o server/base64.o
OBJ_HW = hardware/i2c.o hardware/gpio.o
SHARED = -lpthread

main: hardware server
	$(CC) $(OBJ_SERVER) $(OBJ_HW) main.c $(SHARED) -o DragonWeb

test: hardware server
	$(CC) $(OBJ_SERVER) $(OBJ_HW) test.c $(SHARED) -o Test

hardware: $(OBJ_HW)

hardware/%.o: hardware/%.c
	$(CC) $(CFLAGS) -o $@ $<

server: $(OBJ_SERVER)

server/%.o: server/%.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm hardware/*.o
	rm server/*.o
	rm DragonWeb
	rm Test
