#ifndef CHALLENGE_CLIENT_H
#define CHALLENGE_CLIENT_H

#include <common.h>
#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <socket.h>
#include <commands.h>

void client_loop(int socket_fd, char *const buffer);
void input_id(char *buffer);
bool login(int socket_fd, char *buffer);
void print_status_denied(int fd);
int reading_status(int fd);
int send_command(int fd, struct command_options *options);
int serialize_and_send(int fd, char *buffer, char *fmt, ...);
void reading_list_return(int fd);
void reading_get_return(int fd, struct command_options *options);

#endif