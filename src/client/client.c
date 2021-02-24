#include <common.h>
#include <util.h>
#include <socket.h>
#include <stdio.h>

void* get_in_addr(struct sockaddr* socket_addr);
void client_loop(int socket_fd, char* const buffer);

int main()
{
    int main_connection;
    char buffer[MAX_BUFFER_SIZE];
    main_connection = setup_client();
    
    write(main_connection, HANDSHAKE, MAX_HANDSHAKE_SIZE);
    client_loop(main_connection, buffer);
    
    close(main_connection);
    
    return 0;
}

void client_loop(int socket_fd, char* const buffer)
{   
    int nbytes;
    for(;;)
    {
        bzero(buffer, sizeof buffer);
        nbytes = recv(socket_fd, buffer, MAX_BUFFER_SIZE, 0);
        log_if_err_and_exit(nbytes, "recv");

        if(nbytes == 0)
        {
            fprintf(stderr, "connection closed with server\n");
            break;
        }
        else 
        {
            if(check_hello(buffer) == 0)
            {
                // conectado
                printf("connected to server\n");
                for(;;)
                {

                }
            }
            else 
            {
                fprintf(stderr, "failed to connected");
                break;
            }
            
        }
    }
}


void* get_in_addr(struct sockaddr* socket_addr)
{
    if(socket_addr->sa_family == AF_INET) 
    {
        return &(((struct sockaddr_in*)socket_addr)->sin_addr);
    }

    return &(((struct sockaddr_in6*)socket_addr)->sin6_addr);
}
