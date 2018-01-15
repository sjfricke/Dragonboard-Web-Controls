CC = gcc
CFLAGS = -Wall -Werror -c
SPHINX = -I/usr/local/include -I/usr/local/include/sphinxbase -I/usr/local/include/pocketsphinx -L/usr/local/lib -lpocketsphinx -lsphinxbase -lsphinxad
OBJ_SERVER = server/util.o server/server.o server/websocket.o server/http.o server/sha1.o server/base64.o
OBJ_AUDIO = audio/voice.o audio/commands.o audio/loopback.o audio/sound_clip.o
OBJ_HW = hardware/i2c.o hardware/PCA9685.o  hardware/clock.o hardware/fan.o hardware/gpio.o hardware/gyro.o  hardware/led.o hardware/LSM6DS3H.o  hardware/servo.o
SHARED = -lpthread -lpocketsphinx -lsphinxbase -lsphinxad -lasound

main: hardware audio server
	$(CC) $(OBJ_SERVER) $(OBJ_AUDIO) $(OBJ_HW) main.c $(SPHINX) $(SHARED) -o Seahaven

test: hardware audio server
	$(CC) $(OBJ_SERVER) $(OBJ_AUDIO) $(OBJ_HW) test.c $(SPHINX) $(SHARED) -o TestSeahaven

hardware: $(OBJ_HW)

hardware/%.o: hardware/%.c
	$(CC) $(CFLAGS) -o $@ $<

audio: $(OBJ_AUDIO)

audio/%.o: audio/%.c
	$(CC) $(SPHINX) -c -o $@ $<

server: $(OBJ_SERVER)

server/%.o: server/%.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm hardware/*.o
	rm audio/*.o
	rm server/*.o
	rm Seahaven
	rm TestSeahaven

# Audio test make
audioTest:
	$(CC) -Wall -Werror audio/playTest.c -lpthread -lasound -o TestSeahaven
