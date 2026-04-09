CC = gcc
CFLAGS = -Wall -Wextra -g

all: server client

server: server.c
	$(CC) $(CFLAGS) -o server server.c

client: client.c
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm -f server client server.log /tmp/server_fifo /tmp/client_fifo_*