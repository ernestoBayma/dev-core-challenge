#ifndef CHALLENGE_COMMON_H
#define CHALLENGE_COMMON_H

#define _GNU_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netdb.h>
#include <limits.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>

#define PORT "50080"
#define MAX_BUFFER_SIZE 8192
#define MAX_HANDSHAKE_SIZE 6
#define HANDSHAKE "hello"
#define SIZE_STATUS 4
#define STATUS_SUCCESS "OOK"
#define STATUS_DENIED "NOK"
#define MAX_USERNAME_LEN 256
#define MAX_DIR_LEN PATH_MAX
#define MAX_USERS 2
struct user
{
    /*
    *   username tem o limite de MAX_USERNAME_LEN e pode apenas conter caracteres alfanuméricos
    */
    char username[MAX_USERNAME_LEN];
    /* 
    *  directory é o diretório onde está salvo
    *  os arquivos do usuário, e ele é composto pelo cwd + nome do usuário
    *  NOTE: como os usuários são estáticos e não mudam e não são retirados, não é necessário usar free
    */
    char* directory;    
};

// lista de comandos que são partilhados entre o cliente e servidor

//static char *commands[] = 
//{ 
    /*
    * list é seguido pela opção de ordenação (file ou size) e depois se é em ordem crescente ou decrescente (asc / desc)
    * ex : list file asc -> file a size 20 file b size 2 file c size 2089
    */
  //  "list",
    /*
    * get é seguido pelo nome do arquivo(file) que está na pasta do usuário e manda o conteúdo para o client do usuário que salva a pasta raiz
    * ex: get a -> file a
    */
    //"get",
    /*
    * send é seguido pelo nome do arquivo(file) e manda para a pasta do usuário
    * ex: send b -> /username/b
    */
   // "send" 
//};



// TODO(ernesto): Mudar para uma função que logue e não saia
void log_if_err_and_exit(int value, char* message);

// tenta escrever tudo do buffer se um erro ocorrer retorna -1
// até o erro
int write_to_socket(int socket_fd, void* buffer, size_t buffer_len);

// tenta mandar tudo do buffer se um erro ocorrer retorna -1
int send_all(int socket_fd, char *buffer, int *buffer_len);




#endif