#include <common.h>
#include <socket.h>
#include <stdio.h>

int setup_server()
{
    struct addrinfo hints;
    struct addrinfo *result, *traversal;
    int status, socket_fd;
    int yes = 1;

    memset(&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    status = getaddrinfo(NULL, PORT, &hints, &result);
    if(status != 0) 
    {
        // TODO(ernesto) Trocar para uma função de log
        fprintf(stderr, "getaddrinfo error %s\n", gai_strerror(status));
        return -1;
    }

    for(traversal = result; traversal != NULL; traversal = traversal->ai_next)
    {
        socket_fd = socket(traversal->ai_family, traversal->ai_socktype, traversal->ai_protocol);
        if(socket_fd < 0)
        {
            continue;
        }

        setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        status = bind(socket_fd, traversal->ai_addr, traversal->ai_addrlen);
        if(status == 0)
        {
            break;
        }

        close(socket_fd);
    }

    if(traversal == NULL) 
    {
        fprintf(stderr, "server: could not bind\n");
        return -1;
    }

    freeaddrinfo(result);

    log_if_err_and_exit(listen(socket_fd, SOMAXCONN), "server: listen");

    return socket_fd;
}

