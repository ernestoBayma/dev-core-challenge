#include <common.h>
#include <util.h>
#include <socket.h>
#include <sys/stat.h>
#include <stdio.h>

static struct user users[MAX_USERS];

// uma simples linear search 
// retorna o index do usuário na array users
// se não achar retorna -1
int find_user(char* username);
void make_users_dirs();
void handle_client(int fd, char* buffer);



int main()
{
    int listener, new_conection, nbytes;
    //char client_ip_str[INET6_ADDRSTRLEN];
    char buffer[MAX_BUFFER_SIZE] = {0};

    struct sockaddr_in sock_addr;
    socklen_t sock_addr_len;

    add_user(&(users[0]), "bob");
    add_user(&(users[1]), "test");
    make_users_dirs();
    
    listener = setup_server();
    log_if_err_and_exit(listener, "server: setup_server");


    while(1)
    {
        sock_addr_len = sizeof sock_addr;
        if((new_conection = accept(listener, (struct sockaddr*)&sock_addr, &sock_addr_len)) < 0)
        {
            perror("accept");
            exit(1);
        }

        bzero(buffer, sizeof buffer);
        if((nbytes = recv(new_conection, buffer, sizeof buffer, 0)) < 0)
        {
            perror("recv");
            exit(1);
        }

        if(nbytes == 0)
        {
            printf("ending connection\n");
            close(new_conection);
            continue;
        }
        else
        {
            if(check_hello(buffer) == 0)
            {
                // process
                printf("sending hello back\n");
                write(new_conection, HANDSHAKE, MAX_HANDSHAKE_SIZE);
                
                handle_client(new_conection, buffer);
            }
            else 
            {
                printf("client did not send hello\n");
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
int find_user(char* username)
{
    int result = -1;
    for(int i = 0; i < MAX_USERS; i++)
    {
        if(strcmp(users[i].username, username) == 0)
        {
            result = i;
        }
    }

    return result;
}

void handle_client(int fd, char* buffer)
{
    bzero(buffer, sizeof buffer);
    int nbytes;
    // vê se tem algum user com esse user id
    nbytes = recv(fd, buffer, sizeof buffer, 0);
    log_if_err_and_exit(nbytes, "recv");
    if(nbytes == 0)
    {
        printf("connection closed\n");
        close(fd);
    }
    else
    {
        // TODO(ernesto): validar no server também
        if(find_user(buffer) >= 0)
        {
            printf("found user\n");
            nbytes = write(fd, STATUS_SUCCESS, SIZE_STATUS);
            log_if_err_and_exit(nbytes, "write");
        }
        else 
        {
            printf("did not find user\n");
            nbytes = write(fd, STATUS_DENIED, SIZE_STATUS);
            log_if_err_and_exit(nbytes, "write");
        }
    }
}

void send_status_denied(int fd, char* message)
{
    int len = strlen(message);
    char buffer[MAX_BUFFER_SIZE];
    int written = snprintf(buffer, sizeof buffer, "%d", len);
    log_if_err_and_exit(write(fd, buffer, written), "write sending len on send_status_denied");
    log_if_err_and_exit(write(fd, message, len), "write sending the message on send_status_denied");
}

void make_users_dirs()
{
    for(int i = 0; i < MAX_USERS; i++)
    {
        mkdir(users[i].directory, 0777);
        if(errno == EEXIST) continue;
    }
}

