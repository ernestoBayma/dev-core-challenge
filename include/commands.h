
/* 
Esses parsers são bem simples e rígidos sobre a forma que
interpreta um input, mas servem pro propósito
eles não ignoram o tamanho de espaço que o user coloca, então 
espaço entre parte de um comando tem que ser apenas de um.
*/ 



#ifndef CHALLENGE_COMMANDS_H
#define CHALLENGE_COMMANDS_H
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define List_C (1 << 3) // comando principal list
#define Get_C  (1 << 7) // comando principal get
#define Send_C (1 << 11) // comando principal send
#define Help_C (1 << 15) // comando principal help, server ignora
#define Exit_C (1 << 19) // comando principal exit


struct command_options
{
    char* path;
    char* filename;
    int bitfield;
    bool asc; // se asc for falso então desc é verdade
    bool name; // se name for false então size é verdade
    size_t file_size; // só é usado no comando send
};
struct status;

void help();
bool parse_command_from_client(char *command, struct command_options *options);
bool parse_commands_from_user(char *buffer, struct command_options *options, struct status *s);
int read_command(int fd, char *buffer);
void handle_list_command(int fd, int user_index, struct command_options *options);



#endif