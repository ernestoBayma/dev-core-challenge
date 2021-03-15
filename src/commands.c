#include <common.h>
#include <commands.h>


// espera receber um buffer contendo input do usuário
// retorna true se for um comando válido e preenche options
// com as opções do usuário
// retorna falso se não for um comando reconhecido 
bool parse_commands_from_user(char *buffer, struct command_options *options, struct status *s)
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
    options->name = false;
    options->asc = false;
    options->file_size = 0;

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
                else
                {
                    return false;
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
            else
            {
                indicate_error(s, "path inválido.\n");
                return true;
            }

            struct stat st;
            if(stat(options->path, &st) == 0)
            {
                if(!S_ISREG(st.st_mode))
                {
                    indicate_error(s, "path não tem um arquivo\n");
                    return true;
                }
                options->file_size = st.st_size;

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

// espera que o cliente mande o tamanho da string do comando primeiro
// e depois o comando de forma serializada como uma string
int read_command(int fd, char *buffer)
{   
    bzero(buffer, MAX_BUFFER_SIZE);
    int command_size;
    int nbytes;
    // recebe o tamanho do comando
    nbytes = read(fd, &command_size, sizeof command_size);
    command_size = ntohl(command_size);
    if(nbytes < 0)
    {
        if(errno == EBADF)
        {
            return 0;
        }

        log_if_err_and_exit(nbytes, "read: command size");
    }
    


    // recebe um comando
    nbytes = recv_and_fill_buffer(fd, buffer, command_size);
    if(nbytes < 0)
    {
        if(errno == EBADF || errno == EFAULT )
        {
            return 0;
        }
        log_if_err_and_exit(nbytes, "recv: command");
    }
    
    return nbytes;
}

// lê o comando em forma de uma string
bool parse_command_from_client(char *command, struct command_options *options)
{   
    int cursor = 0;
    if(options == NULL)
    {
        return false;
    }
    options->bitfield = 0;
    options->path = NULL;
    options->name = NULL;
    options->name = false;
    options->asc = false;

    if(strncmp(command, "list", 4) == 0)
    {
        options->bitfield |= List_C;
        cursor += 4;
        if(strncmp(command + cursor, " ", 1) == 0)
        {
            cursor += 1;
            if(strncmp(command + cursor, "0", 1) == 0)
            {
                options->name = false;
            }
            else if(strncmp(command + cursor, "1", 1) == 0) 
            {
                options->name = true;
            }

            cursor += 1;

            if(strncmp(command + cursor, " ", 1) == 0)
            {
                cursor += 1;
                if(strncmp(command + cursor, "0", 1) == 0)
                {
                    options->asc = false;
                    return true;
                }
                else if(strncmp(command + cursor, "1", 1) == 0)
                {
                    options->asc = true;
                    return true;
                }
            }

        }
        
    }
    else if(strncmp(command, "send", 4) == 0)
    {
        options->bitfield |= Send_C;
        cursor += 4;
        if(strncmp(command + cursor, " ", 1) == 0)
        {
            cursor += 1;
            // assumimos que depois do espaço tem um path, mas não fazemos 
            // validação ainda
            int len = strlen((command + cursor));
            char *path = malloc(sizeof(char) * len + 1);
            strcpy(path, command + cursor);
            options->path = path;
            return true;
        }
    }
    else if(strncmp(command, "get", 3) == 0)
    {
        options->bitfield |= Get_C;
        cursor += 3;
        if(strncmp(command + cursor, " ", 1) == 0)
        {
            cursor += 1;
            // assumimos que depois do espaço tem um filename, mas não fazemos 
            // validação ainda
            int len = strlen(command + cursor);
            char *filename = malloc(sizeof(char) * len + 1);
            strcpy(filename, command + cursor);
            options->filename = filename;
            return true;
        }
    }
    else if(strncmp(command, "exit", 4) == 0)
    {
        options->bitfield |= Exit_C;
        return true;
    }

    // não deveria ocorrer
    return false;    
}