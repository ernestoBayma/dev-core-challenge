#ifndef CHALLENGE_SERVER_H
#define CHALLENGE_SERVER_h

#include <common.h>
#include <socket.h>
#include <commands.h>


typedef int user_t;
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




struct directory_files
{
    char filename[NAME_MAX];
    size_t size;
    struct directory_files *next;
};


// uma simples linear search 
// retorna o index do usuário na array users
// se não achar retorna -1
int find_user(char* username);
void add_user(struct user *p_user, char *username);
void make_users_dirs();
void handle_client(int fd, char* buffer);

void send_status_denied_and_motive(int fd, char *message);
void send_status_denied(int fd);
void send_status_success(int fd);
int read_command(int fd, char *buffer);
void send_formated_list_command(int fd, char *buffer, struct directory_files *dir_files);
void handle_get_command(int fd, int user_index, struct command_options *options);
void handle_send_command(int fd, int user_index, struct command_options *options);
void send_file(int fd, int file_fd, size_t file_size);
size_t check_file(int fd, int user_index, char* buffer, struct command_options *options);

/* files */
struct directory_files *new_directory_file(char *filename, size_t size);
void append_to_root(struct directory_files **root, struct directory_files *new_node);
void free_directory_files(struct directory_files **root);
void print_directory_files(struct directory_files *root);
struct directory_files* get_all_files_from_directory(char *dirpath, struct status *s, bool by_name, bool desc);

void split_directory_files(struct directory_files* root,
                                              struct directory_files **front,
                                              struct directory_files **middle);
void sort_directory_files(struct directory_files **root, bool by_name, bool desc);
struct directory_files *merge(struct directory_files *a, struct directory_files *b, bool by_name, bool desc);
struct directory_files *reverse(struct directory_files *root);
bool search(struct directory_files *files, char *filename);


#endif