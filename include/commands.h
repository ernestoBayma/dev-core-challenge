#ifndef CHALLENGE_COMMANDS_H
#define CHALLENGE_COMMANDS_H
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define List_C (1 << 3) // comando principal list
#define Get_C  (1 << 7) // comando principal get
#define Send_C (1 << 11) // comando principal send
#define Help_C (1 << 15) // comando principal help, server ignora
#define Exit_C (1 << 19) // comando principal exit, server ignora


struct command_options
{
    char* path;
    char* filename;
    int bitfield;
    bool asc; // se asc for falso então desc é verdade
    bool name; // se name for false então size é verdade
};

// espera receber um buffer contendo input do usuário
// retorna true se for um comando válido e preenche options
// com as opções do usuário
// retorna falso se não for um comando reconhecido 
bool parse_commands(char *buffer, struct command_options *options)
{
    // assume que buffer é termina com null
    char *temp = buffer;
    int len = strlen(temp);
    int current_index = 0;
    // remove o new line
    if(temp[len - 1] == '\n')
    {
        temp[len - 1] = '\0';
    }
    options->bitfield = 0;
    options->path = NULL;
    options->name = NULL;

    if(strncmp(temp, "list", 4) == 0 || strncmp(temp, "LIST", 4) == 0)
    {
        options->bitfield |= List_C;
        current_index+= 4;
        if(strncmp(temp + current_index, " ", 1) == 0)
        {
            current_index++;
            if(strncmp(temp + current_index, "name", 4) == 0 ||
               strncmp(temp + current_index, "NAME", 4) == 0)
            {
                options->name = true;
                current_index += 4;
            }
            else if(strncmp(temp + current_index, "size", 4) == 0 ||
                    strncmp(temp + current_index, "SIZE", 4) == 0 )
            {
                options->name = false;
                current_index += 4;
            }
            else 
            {
                options = NULL;
                return false;
            }

            if(strncmp(temp + current_index, " ", 1) == 0)
            {
                current_index++;
                if(strncmp(temp + current_index, "asc", 3) == 0 ||
                   strncmp(temp + current_index, "ASC", 3) == 0)
                {
                    options->asc = true;
                    return true;
                }
                else if(strncmp(temp + current_index, "desc", 4) == 0 ||
                        strncmp(temp + current_index, "DESC", 4) == 0)
                {
                    options->asc = false;
                    return true;
                }
            } 
        }
    }
    else if(strncmp(temp, "get", 3) == 0 ||
            strncmp(temp, "GET", 3) == 0)
    {
        options->bitfield |= Get_C;
        current_index += 3;
        if(temp[current_index] == ' ')
        {
            current_index+= 1;

            temp += current_index;
            // assume que daqui até o byte null pode ter um nome de arquivo
            int len = strlen(temp);
            // malloca memoria que tem que ser liberada depois
            char *filename = malloc(len + 1);
            strncpy(filename, temp, len);
            // coloca null byte;
            filename[len + 1] = '\0';
            options->filename = filename;

            return true;
        }
    }
    else if(strncmp(temp, "send", 4) == 0 ||
            strncmp(temp, "SEND", 4) == 0)
    {
        current_index+= 4;

        options->bitfield |= Send_C;
        if(temp[current_index] == ' ')
        {
            current_index += 1;

            temp += current_index;
            
            // NOTE(ernesto): essa memória tem que ser livre depois que usar-la
            char* resolved;
            if((resolved = realpath(temp, NULL)) != NULL)
            {
                options->path = resolved;
            }
            return true;
        }
        
    }
    else if(strcmp(temp, "exit") == 0 ||
            strcmp(temp, "EXIT") == 0)
    {
        options->bitfield |= Exit_C;
        return true;
    }
    else if(strcmp(temp, "help") == 0 ||
            strcmp(temp, "HELP") == 0)
    {
        options->bitfield |= Help_C;
        return true;
    }

    return false;
}



#endif