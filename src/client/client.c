#include "client.h"

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

void client_loop(int fd, char* const buffer)
{   
    int nbytes;
    bool running = false;
    struct command_options options = {0};
    for(;;)
    {
        bzero(buffer, sizeof buffer);
        nbytes = recv(fd, buffer, MAX_BUFFER_SIZE, 0);
        log_if_err_and_exit(nbytes, "recv");

        if(nbytes == 0)
        {
            fprintf(stderr, "conexão com o server fechou.\n");
            break;
        }
        else 
        {
            if(check_hello(buffer) == 0)
            {
                // conectado
                printf("conectado ao servidor.\n");
                running = true;

                if(!login(fd, buffer))
                {
                    print_status_denied(fd);
                    break;
                }
                while(running)
                {
                    bzero(buffer, sizeof buffer);

                    printf(">");
                    fgets(buffer, MAX_BUFFER_SIZE - 1, stdin);
                    
                    if(parse_commands_from_user(buffer, &options))
                    {
                        // comandos tem que ser enviados para o servidor
                        // menos help
                        // e o servidor manda o OOK se for válido ou
                        // NOK se for inválido seguido por o tamanho em bytes
                        // da resposta e o conteúdo da resposta para ser impresso na tela
                        
                
                        if(options.bitfield & Help_C)
                        {
                            help();
                        }
                        else
                        {
                            if(send_command(fd, &options) != -1)
                            {
                                int status_result = reading_status(fd);
                                if( status_result > 0)
                                {
                                    if(options.bitfield & List_C)
                                    {
                                        reading_list_return(fd);
                                    }
                                    else if(options.bitfield & Get_C)
                                    {
                                        reading_get_return(fd, &options);
                                    }
                                    else if(options.bitfield & Exit_C)
                                    {
                                        running = false;
                                    }
                                }
                                else if(status_result < 0)
                                {
                                    print_status_denied(fd);
                                }
                                else
                                {
                                    fprintf(stderr, "não reconhecido. buffer %s.\n", buffer);
                                }
                            }
                            else
                            {
                                printf("erro ao mandar o comando %s\n", buffer);
                                exit(1);
                            }
                        }
                    }
                    else 
                    {
                        help();
                    }

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
                } 

                
                // sair do loop principal
                break;
            }
            else 
            {
                fprintf(stderr, "falha ao se conectar\n");
                break;
            }
            
        }
    }
}

void help()
{
    printf("help: lista opções disponíveis\n");
    printf("list <name|size> <asc|desc>: lista os arquivos da pasta do usuário\n");
    printf("                             ordenados por nome ou tamanho\n");
    printf("                             crescente(asc) ou decrescente(desc).\n");
    printf("send <caminho para arquivo>: envia um arquivo do caminho do arquivo para\n");
    printf("                             a pasta do usuário\n");
    printf("get <arquivo>: recupera o arquivo da pasta do usuário\n");
    printf("exit: termina conexão com o servidor e fecha o cliente.\n");
}


int send_command(int fd, struct command_options *options)
{
    if(options == NULL)
    {
        return -1;
    }
    char buffer[MAX_BUFFER_SIZE];
    bzero(buffer, MAX_BUFFER_SIZE);
    if(options->bitfield & List_C)
    {   
        return serialize_and_send(fd, buffer, "list %d %d", options->name, options->asc);
    }
    else if(options->bitfield & Send_C)
    {
        return serialize_and_send(fd, buffer, "send %s", options->path);
    }
    else if(options->bitfield & Get_C)
    {
        return serialize_and_send(fd, buffer, "get %s", options->filename);
    }
    else if(options->bitfield & Exit_C)
    {
        return serialize_and_send(fd, buffer, "exit");
    }
    else
    {
        return -1;
    }
}

void reading_get_return(int fd, struct command_options *options)
{
    
    int size_of_response;
    int to_read, wbytes = 0;
    off_t offset = 0;
    int rbytes = recv(fd, &size_of_response, sizeof size_of_response, 0);
    log_if_err_and_exit(rbytes, "recv");
    size_of_response = ntohl(size_of_response);

    char buffer[MAX_BUFFER_SIZE];
    bzero(buffer, sizeof buffer);

    int file_fd = open(options->filename, O_WRONLY | O_APPEND | O_CREAT, 0766);
    if(file_fd == -1)
    {
        perror("erro abrindo o arquivo.");
        return;
    }
    offset = lseek(file_fd, 0, SEEK_CUR);
    if(offset == -1)
    {
        printf("erro pegando o offset do arquivo.\n");
        return;
    }


    while(size_of_response > 0)
    {
        to_read = min(MAX_BUFFER_SIZE, size_of_response);
        rbytes = read(fd, buffer, to_read);

        if(rbytes == -1)
        {
            printf("erro lendo a reposta do arquivo.\n");
            return;
        }
        if(rbytes == 0)
        {
            break; // final do arquivo
        }

        wbytes = write(file_fd, buffer, rbytes);
        if(wbytes == -1)
        {
            printf("error escrevendo o arquivo.\n");
            return;
        }

        size_of_response -= wbytes;
    }

    if(lseek(file_fd, offset, SEEK_SET) == -1)
    {
        printf("error colocando o offset no lugar.\n");
        return;
    }
}


void reading_list_return(int fd)
{
    int size_of_response;
    int nbytes = recv(fd, &size_of_response, sizeof size_of_response, 0);
    log_if_err_and_exit(nbytes, "recv");
    size_of_response = ntohl(size_of_response);

    
    char buffer[MAX_BUFFER_SIZE];
    bzero(buffer, sizeof buffer);
    recv_and_fill_buffer(fd, buffer, size_of_response);
    printf("%s", buffer);
}

int reading_status(int fd)
{
    char temp_buffer[MAX_BUFFER_SIZE] = {0};
    int nbytes;
    
    nbytes = recv_and_fill_buffer(fd, temp_buffer, SIZE_STATUS);
    log_if_err_and_exit(nbytes, "recv: check_status_ok");

    if(strcmp(temp_buffer, STATUS_SUCCESS) == 0)
    {
        return 1;
    }
    else if(strcmp(temp_buffer, STATUS_DENIED) == 0)
    {
        return -1;
    }
    return 0;
}


void print_status_denied(int fd)
{
    char buffer[MAX_BUFFER_SIZE] = {0};
    int nbytes;

    int32_t size_of_response;
    nbytes = read(fd, &size_of_response, sizeof(size_of_response));
    size_of_response = ntohl(size_of_response);

    int bytes_read;
    bzero(buffer, sizeof buffer);
    do 
    {
        nbytes = recv(fd, buffer, MAX_BUFFER_SIZE, 0);
        if(nbytes == -1) 
        {
            if(errno == EINTR) continue;
            log_if_err_and_exit(nbytes, "recv");
        }
        // só sai do loop?
        if(nbytes == 0)
        {
            break;
        }
        bytes_read += nbytes;
        printf("%s", buffer);

    } while(bytes_read < size_of_response);
    printf("\n");
}

bool login(int socket_fd, char* buffer)
{
    int nbytes;
    bzero(buffer, sizeof buffer);
                
    input_id(buffer);

    // manda o user id
    write(socket_fd, buffer, MAX_USERNAME_LEN);
    
    bzero(buffer, sizeof buffer);
    // recebe o status code
    nbytes = recv(socket_fd, buffer, SIZE_STATUS, 0);
    log_if_err_and_exit(nbytes, "recv");
    
    if(nbytes == 0)
    {
        printf("conexão com o servidor foi fechada.\n");
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
                printf("id inválida.\n");
                printf("id não pode ser maior que 200 caracteres.\n");
                continue;
            }
            
            for(int i = 0; i < curr_len; i++)
            {
                if(!isalnum(buffer[i]))
                {
                    printf("id inválida.\n");
                    printf("uma id válida é composta apenas de caracteres alfanuméricos.\n");
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
