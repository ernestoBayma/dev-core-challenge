#include <common.h>
#include <util.h>
#include <socket.h>
#include <commands.h>
#include "client.h"

#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

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
    bool running = false;
    struct command_options options = {0};
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
                running = true;

                if(!login(socket_fd, buffer))
                {
                    break;
                }
                while(running)
                {
                    bzero(buffer, sizeof buffer);

                    fgets(buffer, MAX_BUFFER_SIZE - 1, stdin);
                    
                    if(parse_commands(buffer, &options))
                    {
                        if(options.bitfield & List_C)
                        {
                            printf("list command name:%d size:%d asc: %d desc: %d\n", options.name, !options.name, options.asc, !options.asc);
                        }
                        else if(options.bitfield & Send_C)
                        {
                            if(options.path)
                            {
                                printf("send command path: %s\n", options.path);
                            }
                            
                        }
                        else if(options.bitfield & Get_C)
                        {
                            if(options.filename)
                            {
                                printf("get command filename: %s\n", options.filename);
                            }
                            
                        }
                        else if(options.bitfield & Exit_C)
                        {
                            // sai do loop 
                            running = false;
                        }
                        else if(options.bitfield & Help_C)
                        {
                            printf("help command\n");
                        }
                    }
                    else 
                    {
                        // help
                        printf("not a valid command\n");
                    }
                } 

                // saimos do event loop
                // limpa tudo
                if(options.filename != NULL)
                {
                    free(options.filename);
                    options.filename = NULL;
                }
                if(options.path != NULL)
                {
                    free(options.path);
                    options.path = NULL;
                }
                // sair do loop principal
                break;
            }
            else 
            {
                fprintf(stderr, "failed to connected\n");
                break;
            }
            
        }
    }


    
    
}

bool login(int socket_fd, char* buffer)
{
    int nbytes;
    bzero(buffer, sizeof buffer);
                
    input_id(buffer);

    // manda o user id
    write(socket_fd, buffer, MAX_USERNAME_LEN);

    // recebe o status code
    nbytes = recv(socket_fd, buffer, SIZE_STATUS, 0);
    log_if_err_and_exit(nbytes, "recv");
    
    if(nbytes == 0)
    {
        printf("connnection with the server was closed\n");
        exit(1);
    }
    else
    {
        if(strcmp(buffer, STATUS_SUCCESS) == 0)
        {
            return true;
        }
        else if(strcmp(buffer, STATUS_DENIED) == 0) 
        {
            return false;
        }
        else 
        {
            // should never be here
            return false;
        }
    }
}


void input_id(char* buffer)
{
    bool valid_id = true;
    while(true)
    {
        printf("what is your id: ");
        if(fgets(buffer, sizeof buffer, stdin) != NULL)
        {
            int new_line = strlen(buffer);
            buffer[new_line - 1] = '\0';
            int curr_len = strlen(buffer);

            if(curr_len > MAX_USERNAME_LEN)
            {
                printf("not a valid id.\n");
                printf("ids cannot be bigger than 256 characters.\n");
                continue;
            }
            
            for(int i = 0; i < curr_len; i++)
            {
                if(!isalnum(buffer[i]))
                {
                    printf("not a valid id.\n");
                    printf("a valid id is made only of alphanumeric characters.\n");
                    valid_id = false;
                    break;
                }
                // até agora é uma id válida
                valid_id = true;
            }

            // se sairmos do loop com valid_id == true então é uma id válida
            if(valid_id) 
            {
                break;
            }
        }
    }
}
