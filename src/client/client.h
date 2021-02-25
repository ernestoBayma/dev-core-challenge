#ifndef CHALLENGE_CLIENT_H
#define CHALLENGE_CLIENT_H



void client_loop(int socket_fd, char *const buffer);
void input_id(char *buffer);
bool login(int socket_fd, char *buffer);

#endif