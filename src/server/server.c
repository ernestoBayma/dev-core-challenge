#include "server.h"
#include <syslog.h>
#include <stdio.h>

static struct user users[MAX_USERS];

static void handle_sigchld(int sig)
{
    int savederrno;
    savederrno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0) continue;
    errno = savederrno;
}

int became_daemon()
{
    
}


int main()
{
    int listener, new_conection, nbytes;
    char buffer[MAX_BUFFER_SIZE] = {0};
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = handle_sigchld;

    if(sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        syslog(LOG_ERR, "Error from sigaction %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in sock_addr;
    socklen_t sock_addr_len;

    add_user(&(users[0]), "bob");
    add_user(&(users[1]), "test");
    make_users_dirs();

    listener = setup_server();
    syslog(LOG_ERR, "server: setup_server %s", strerror(errno));

    while (1)
    {
        sock_addr_len = sizeof sock_addr;
        if ((new_conection = accept(listener, (struct sockaddr *)&sock_addr, &sock_addr_len)) < 0)
        {
            syslog(LOG_ERR, "cannot accept %s", strerror(errno));
            exit(1);
        }

        bzero(buffer, sizeof buffer);
        if ((nbytes = recv(new_conection, buffer, sizeof buffer, 0)) < 0)
        {
            syslog(LOG_ERR, "cannot read from new_connection %s", strerror(errno));
            exit(1);
        }

        if (nbytes == 0)
        {
            syslog(LOG_INFO , "ending connection.");
            close(new_conection);
            continue;
        }
        else
        {
            if (check_hello(buffer) == 0)
            {
                // process
                printf("sending hello back\n");
                write(new_conection, HANDSHAKE, MAX_HANDSHAKE_SIZE);

                // NOTE(ern): Essa é uma forma bem simplistica de lidar com vários 
                // clientes
                switch(fork())
                {
                    case -1:
                    syslog(LOG_ERR, "Error creating a child %s", strerror(errno));
                    close(new_conection);
                    break;
                    case 0: // novo processo
                    close(listener);
                    handle_client(new_conection, buffer);
                    _exit(EXIT_SUCCESS);
                    default:
                    close(new_conection);
                    break;
                }
            }
            else
            {
                syslog(LOG_ERR, "client did not send hello.");
                close(new_conection);
            }
        }
    }

    close(listener);
    return 0;
}

// uma simples linear search
// retorna o index do usuário na array users
// se não achar retorna -1
user_t find_user(char *username)
{
    user_t result = -1;
    for (int i = 0; i < MAX_USERS; i++)
    {
        if (strcmp(users[i].username, username) == 0)
        {
            result = i;
        }
    }

    return result;
}

void handle_client(int fd, char *buffer)
{
    int nbytes;

    // vê se tem algum user com esse user id
    nbytes = recv(fd, buffer, MAX_USERNAME_LEN, 0);
    log_if_err_and_exit(nbytes, "recv");
    if (nbytes == 0)
    {
        printf("connection closed\n");
        close(fd);
    }
    else
    {
        user_t user_index = find_user(buffer);
        if (user_index >= 0)
        {
            printf("found user\n");
            send_status_success(fd);

            while (1)
            {
                struct command_options options = {0};

                nbytes = read_command(fd, buffer);
                if (nbytes == 0)
                {
                    // sair da thread
                    printf("conexão foi fechada pelo cliente\n");
                    close(fd);
                    break;
                }
                else if (nbytes > 0)
                {
                    if (parse_command_from_client(buffer, &options))
                    {
                        if (options.bitfield & List_C)
                        {
                            handle_list_command(fd, user_index, &options);
                        }
                        else if (options.bitfield & Get_C)
                        {
                            handle_get_command(fd, user_index, &options);
                        }

                        else if (options.bitfield & Send_C)
                        {
                            handle_send_command(fd,user_index, &options);
                        }

                        else if (options.bitfield & Exit_C)
                        {
                            send_status_success(fd);
                            close(fd);
                            break;
                        }
                    }
                    else
                    {
                        printf("erro recebendo o comando. bytes %d\n", nbytes);
                        printf("buffer está:%s", buffer);
                        close(fd);
                        exit(1);
                    }
                }

                // isso tem que ser dentro do loop
                // depois que houver o
                // processamento do comando.
                if (options.path)
                {
                    free(options.path);
                    options.path = NULL;
                }
                if (options.filename)
                {
                    free(options.filename);
                    options.filename = NULL;
                }
            }
        }
        else
        {
            nbytes = write(fd, STATUS_DENIED, SIZE_STATUS);
            log_if_err_and_exit(nbytes, "write");
            send_status_denied_and_motive(fd, "usuário não foi encontrado");
        }
    }
}

void handle_send_command(int fd, int user_index, struct command_options *options)
{
    if(options->path != NULL)
    {
        int32_t size_of_file;
        char buffer[MAX_BUFFER_SIZE] = {0};
        int nbytes, wbytes, to_read;

        send_status_success(fd);
        nbytes = read(fd, &size_of_file ,sizeof(size_of_file));

        if(nbytes <= 0)
        {
            // TODO(ern): mudar para algum tipo de log.
            printf("erro lendo file_size\n");
            return;
        }

        size_of_file = ntohl(size_of_file);

        char *user_dir = users[user_index].directory;
        char *filename_from_path = basename(options->path);

        // NOTE(ern): Usando buffer de forma temporária 
        // para ter o path aonde o arquivo vindo
        // do cliente vai ser salvo 
        snprintf(buffer, PATH_MAX - 1, "%s/%s", user_dir, filename_from_path);

        int file_fd = open(buffer, O_WRONLY | O_CREAT, 0766);
        if(file_fd == -1)
        {
            send_status_denied_and_motive(fd, "erro salvando arquivo na pasta do usuário.");
            return;
        }

        while(size_of_file > 0)
        {
            to_read = min(MAX_BUFFER_SIZE, size_of_file);
            nbytes = read(fd, buffer, to_read);

            if(nbytes == -1)
            {
                printf("erro lendo a reposta do arquivo.\n");
                return;
            }
            if(nbytes == 0)
            {
                break; // final do arquivo
            }

            wbytes = write(file_fd, buffer, nbytes);
            if(wbytes == -1)
            {
                printf("error escrevendo o arquivo.\n");
                close(file_fd);
                return;
            }

            size_of_file -= wbytes;
        }

        close(file_fd);
    }
}


void handle_get_command(int fd, int user_index, struct command_options *options)
{
    
    int file_fd = -1;
    char buffer[PATH_MAX] = {0};
    int file_size = 0;
    if((file_size = check_file(fd, user_index, buffer, options)) > 0)
    {
        printf("get command path: %s\n", buffer);
        file_fd = open(buffer, O_CLOEXEC, O_RDONLY);

        if (file_fd == -1)
        {
            send_status_denied_and_motive(fd, "erro abrindo arquivo.");
        }
        else
        {
            send_file(fd, file_fd, file_size);
        }
    }
           
    
    close(file_fd);
}

size_t check_file(int fd, int user_index, char* buffer, struct command_options *options)
{
    
    struct directory_files *dir_files = NULL;
    char *directory = users[user_index].directory;
    struct stat st;
    struct status s;
    dir_files = get_all_files_from_directory(directory, &s, true, false);
    if (s.error)
    {
        send_status_denied_and_motive(fd, s.message_error);
    }
    else if (dir_files == NULL)
    {
        send_status_denied_and_motive(fd, "pasta do usuário está vazia.");    
    }
    else if (search(dir_files, options->filename))
    {
        snprintf(buffer, PATH_MAX - 1, "%s/%s", directory, options->filename);
        buffer[PATH_MAX - 1] = '\0';

        if (stat(buffer, &st) != -1)
        {
            if ((st.st_mode & S_IFMT) == S_IFREG)
            {
                free_directory_files(&dir_files);
                return st.st_size;
            }
        }
        else 
        {
            send_status_denied_and_motive(fd, "erro pegando as estatísticas do arquivo.");
        }
    }
    else
    {
        send_status_denied_and_motive(fd, "esse arquivo não existe.");
    }

    free_directory_files(&dir_files);
    return 0;
}

void send_file(int fd, int file_fd, size_t file_size)
{
    int sbytes = 0;
    int t_sbytes = 0;
    off_t offset = 0;

    send_status_success(fd);
    int converted = htonl(file_size);
    if (write_all(fd, (char*)&converted, sizeof converted) < 0)
    {
        send_status_denied_and_motive(fd, "erro mandando o tamanho do arquivo.");
        return;
    }
    do
    {
        sbytes = sendfile(fd, file_fd, &offset, file_size);
        if (sbytes == -1)
        {
            send_status_denied_and_motive(fd, "erro mandando o arquivo.");
            break;
        }
        printf("enviamos %d bytes\n", sbytes);
        t_sbytes += sbytes;
    } while (t_sbytes < file_size);
}

void handle_list_command(int fd, int user_index, struct command_options *options)
{
    struct directory_files *dir_files = NULL;
    struct status s;
    char *directory = users[user_index].directory;
    dir_files = get_all_files_from_directory(directory, &s, options->name, !(options->asc));

    if (s.error)
    {
        send_status_denied_and_motive(fd, s.message_error);
    }
    else
    {
        char buffer[MAX_BUFFER_SIZE];
        bzero(buffer, MAX_BUFFER_SIZE);
        if (dir_files == NULL)
        {
            send_status_denied_and_motive(fd, "cliente não tem arquivos na pasta.");
        }
        else
        {
            send_status_success(fd);
            send_formated_list_command(fd, buffer, dir_files);
        }
    }

    free_directory_files(&dir_files);
}

void send_formated_list_command(int fd, char *buffer, struct directory_files *dir_files)
{
    struct directory_files *curr = dir_files;
    int size_message = 0;
    int nbytes;
    // vê o tamanho total da lista
    while (curr != NULL)
    {
        size_message += snprintf(NULL, 0, "%s\n", curr->filename);
        curr = curr->next;
    }

    // o null byte
    size_message += 1;
    // manda o tamanho em bytes da lista
    int converted = htonl(size_message);
    write_all(fd, (char*)&converted, sizeof converted);

    curr = dir_files;
    int bytes_written = 0;
    int cursor = 0;

    // NOTE: pode ocorrer de a lista ser mais curta que o ideal
    // por causa que atualmente o buffer que escrevemos ser fixo no valor de MAX_BUFFER_SIZE
    while (curr != NULL && cursor < MAX_BUFFER_SIZE)
    {
        cursor += snprintf(buffer + cursor, MAX_BUFFER_SIZE - cursor, "%s\n", curr->filename);
        curr = curr->next;
    }

    buffer[MAX_BUFFER_SIZE - 1] = '\0';
    do
    {
        nbytes = write(fd, buffer, size_message);
        if (nbytes == -1)
        {
            log_if_err_and_exit(nbytes, "write");
        }
        bytes_written += nbytes;

    } while (bytes_written < size_message);
}

void send_status_denied_and_motive(int fd, char *message)
{
    char buffer[MAX_BUFFER_SIZE] = {0};
    send_status_denied(fd);
    serialize_and_send(fd, buffer, message);
}

void send_status_denied(int fd)
{
    int nbytes = write(fd, STATUS_DENIED, SIZE_STATUS);
    log_if_err_and_exit(nbytes, "write: send_status_denied.");
}

void send_status_success(int fd)
{
    int nbytes;
    
    nbytes = write(fd, STATUS_SUCCESS, SIZE_STATUS);
    log_if_err_and_exit(nbytes, "write: send_status_ok");
}

void add_user(struct user *p_user, char *username)
{
    char *path = malloc(sizeof(char) * MAX_DIR_LEN);
    char cwd[MAX_DIR_LEN];
    if (path != NULL)
    {
        getcwd(cwd, MAX_DIR_LEN);
        strncat(path, cwd, MAX_DIR_LEN);
        size_t len = strlen(path);

        strncpy(p_user->username, username, MAX_USERNAME_LEN - 1);
        p_user->username[MAX_USERNAME_LEN] = '\0';

        if (len >= MAX_DIR_LEN)
        {
            path[MAX_DIR_LEN] = '\0';
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

void make_users_dirs()
{
    for (int i = 0; i < MAX_USERS; i++)
    {
        mkdir(users[i].directory, 0777);
        if (errno == EEXIST)
            continue;
    }
}
