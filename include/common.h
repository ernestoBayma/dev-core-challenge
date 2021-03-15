#ifndef CHALLENGE_COMMON_H
#define CHALLENGE_COMMON_H

#define _GNU_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <limits.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PORT "50080"
#define MAX_BUFFER_SIZE 8192
#define MAX_HANDSHAKE_SIZE 6
#define HANDSHAKE "hello"
#define SIZE_STATUS 3
#define STATUS_SUCCESS "OOK"
#define STATUS_DENIED "NOK"
#define MAX_USERNAME_LEN 256
#define MESSAGE_ERROR_BUFFER_SIZE 255
#define MAX_DIR_LEN PATH_MAX
#define MAX_USERS 2

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _b : _a; })

struct status
{
    char message_error[MESSAGE_ERROR_BUFFER_SIZE];
    bool error;
};

void indicate_error(struct status *s, char *message);

// TODO(ernesto): Mudar para uma função que logue e não saia
void log_if_err_and_exit(int value, char* message);

int recv_and_fill_buffer(int fd, char *buffer, size_t buffer_size);


int write_all(int fd, char *buffer, size_t buffer_len);

// tenta mandar tudo do buffer se um erro ocorrer retorna -1
int send_all(int socket_fd, char *buffer, int *buffer_len);

int check_hello(char* buffer);

int serialize_and_send(int fd, char *buffer, char *fmt, ...);


#endif