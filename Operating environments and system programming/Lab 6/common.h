#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT             "9034"
#define BUFFER_SIZE      512
#define MAX_CLIENTS       30
#define NAME_LEN          32

typedef struct {
    int socket;
    char name[NAME_LEN];
    struct sockaddr_in addr;
} client_t;

#endif