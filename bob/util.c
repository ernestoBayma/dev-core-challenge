#include <common.h>
#include <util.h>

// remover quando virar daemon
#include <stdio.h>


void log_if_err_and_exit(int value, char* message)
{  
    // TODO(ernesto) Trocar para uma função de log
    if(value < 0)
    {
        perror(message);
        exit(1);
    } 
}

int write_to_socket(int socket_fd, void* buffer, size_t buffer_len)
{
    int cursor = 0;
    int bytes_sent;
    while(cursor < buffer_len)
    {
        bytes_sent = write(socket_fd, buffer + cursor, buffer_len - cursor);

        if(bytes_sent < 0)
        {
            if(errno == EAGAIN) return -1;
        } 

        cursor += bytes_sent;
    }

    return cursor;
}

int send_all(int socket_fd, char *buffer, int *buffer_len)
{
    int total = 0;
    int bytes_left = *buffer_len;
    int n;

    while(total < *buffer_len)
    {
        n = send(socket_fd, buffer, bytes_left, 0);
        if(n == -1) break;
        total += n;
        bytes_left -= n;
    }

    *buffer_len = total; // o retorno está aqui

    return n == -1 ? -1 : 0;
}

void add_user(struct user *p_user, char* username)
{
    char *path = malloc(sizeof(char) * MAX_DIR_LEN);
    char cwd[MAX_DIR_LEN];
    if(path != NULL)
    {
        getcwd(cwd, MAX_DIR_LEN);
        strncat(path, cwd, MAX_DIR_LEN);
        size_t len = strlen(path);

        strncpy(p_user->username, username, MAX_USERNAME_LEN - 1);
        p_user->username[MAX_USERNAME_LEN] = '\0';

        if(len >= MAX_DIR_LEN)
        {
            path[len - 1] = '\0';
            p_user->directory = path;
            return;
        }

        
        path[len] = '/';
        strncat(path, p_user->username, MAX_DIR_LEN - 1);
        path[MAX_DIR_LEN] = '\0';

        p_user->directory = path;
       
    }
    else
    {
        printf("out of memory\n");
        exit(1);
    }
    
}

int check_hello(char* buffer)
{
    return strcmp(HANDSHAKE, buffer);
}