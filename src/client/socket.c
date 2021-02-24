#include <common.h>
#include <socket.h>
#include <stdio.h>

int setup_client()
{
    int main_connection;
    int status;
    struct addrinfo hints, *server_info, *traversal;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if((status = getaddrinfo(NULL, PORT, &hints, &server_info)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }

    for(traversal = server_info; traversal != NULL; traversal = traversal->ai_next)
    {
        if((main_connection = socket(traversal->ai_family,
                                     traversal->ai_socktype,
                                     traversal->ai_protocol)) == -1)
        {
            perror("client: socket");
            continue;
        }

        if(connect(main_connection,
                    traversal->ai_addr,
                    traversal->ai_addrlen) == -1)
        {
            close(main_connection);
            perror("client: connect");
            continue;
        }

        break;
    }
    
    if(traversal == NULL)
    {
        fprintf(stderr, "client: failed to connect\n");
        exit(1);
    }
    freeaddrinfo(server_info);

    return main_connection;
}