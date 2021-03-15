#include <common.h>

// remover quando virar daemon
#include <stdio.h>
#include <stdarg.h>


void log_if_err_and_exit(int value, char* message)
{  
    // TODO(ernesto) Trocar para uma função de log
    if(value < 0)
    {
        perror(message);
        exit(1);
    } 
}

int recv_and_fill_buffer(int fd, char *buffer, size_t buffer_size)
{
    size_t cursor = 0;
    int nbytes;
    int left = buffer_size;
    while(left > 0)
    {
        nbytes = recv(fd, buffer + cursor, left, 0);
        if(nbytes == -1) 
        {
            if(errno == EINTR) continue;
            return -1;
        }
        if(nbytes == 0)
        {
            break;
        }

        cursor += nbytes;
        left -= nbytes;
    }

    return cursor;
}

int write_all(int fd, char *buffer, size_t buffer_len)
{
    int cursor = 0;
    int bytes_sent;
    while(cursor < buffer_len)
    {
        bytes_sent = write(fd, buffer + cursor, buffer_len - cursor);

        if(bytes_sent == -1)
        {
            if(errno == EINTR) continue;

            return -1;
        } 
        if(bytes_sent == 0)
        {
            break;
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

void indicate_error(struct status *s, char *message)
{
    bzero(s->message_error, MESSAGE_ERROR_BUFFER_SIZE);
    strncpy(s->message_error, message, MESSAGE_ERROR_BUFFER_SIZE);
    s->error = true;
}

int serialize_and_send(int fd, char *buffer, char *fmt, ...)
{
    int nbytes;
    va_list va;
    va_start(va, fmt);

    vsnprintf(buffer, MAX_BUFFER_SIZE, fmt, va);
    int len = strlen(buffer);
    int converted = htonl(len);

    // mandamos o tamanho do comando primeiro
    nbytes = write(fd, &converted, sizeof converted);
    log_if_err_and_exit(nbytes, "write in send_command:get: size");

    // mandmos o comando
    nbytes = write_all(fd, buffer, len);
    log_if_err_and_exit(nbytes, "write in send_command:get: command");

    va_end(va);
    return nbytes;
}

int check_hello(char* buffer)
{
    return strcmp(HANDSHAKE, buffer);
}