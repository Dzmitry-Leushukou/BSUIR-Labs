CC = gcc
CFLAGS = -Wall -Wextra -pthread
TARGETS = server client

all: $(TARGETS)

server: server.c common.h
	$(CC) $(CFLAGS) -o $@ server.c

client: client.c common.h
	$(CC) $(CFLAGS) -o $@ client.c

clean:
	rm -f $(TARGETS)

.PHONY: all clean