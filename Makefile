all: client server

CC = gcc

INCLUDE = .

# debug
# CFLAGS = -g -Wall -D_DEFAULT_SOURCE -std=c17

# release
CFLAGS = -O3 -Wall -D_DEFAULT_SOURCE -std=c17

server: server.o
	$(CC) -o server server.o
cli/client: client.o
	$(CC) -o client client.o
server.o: server.c server.h base.h
	$(CC) -I$(INCLUDE) $(CFLAGS) -c server.c
client.o: client.c client.h base.h
	$(CC) -I$(INCLUDE) $(CFLAGS) -c client.c

clean:
	-rm client.o server.o
