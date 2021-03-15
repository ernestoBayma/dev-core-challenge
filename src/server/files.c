#include "server.h"
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>


bool search(struct directory_files *files, char *filename)
{
    struct directory_files *curr = files;
    while(curr != NULL)
    {
        if(strcmp(filename, curr->filename) == 0)
        {
            return true;
        }
        curr = curr->next;
    }

    return false;
    
}

struct directory_files *reverse(struct directory_files *root)
{
    if(root == NULL)
    {
        return NULL;
    }
    struct directory_files *prev = NULL;
    struct directory_files *curr = root;
    struct directory_files *next = NULL;

    while(curr != NULL)
    {
        next = curr->next;
        curr->next = prev;
        prev = curr;
        curr = next;
    }

    return prev;
}


void sort_directory_files(struct directory_files **root, bool by_name, bool desc)
{
    struct directory_files *head = *root;
    struct directory_files *a;
    struct directory_files *b;

    if ((head == NULL) || (head->next == NULL))
    {
        return;
    }

    split_directory_files(head, &a, &b);
    sort_directory_files(&a, by_name, desc);
    sort_directory_files(&b, by_name, desc);

    *root = merge(a, b, by_name, desc);
}

void split_directory_files(struct directory_files *root,
                           struct directory_files **front,
                           struct directory_files **middle)
{
    // ponteiro slow no final vai estar um ponteiro antes do meio da lista.
    struct directory_files *fast;
    struct directory_files *slow;
    slow = root;
    fast = root->next;

    while (fast != NULL)
    {
        fast = fast->next;
        if (fast != NULL)
        {
            slow = slow->next;
            fast = fast->next;
        }
    }

    *front = root;
    *middle = slow->next;
    slow->next = NULL;
}

struct directory_files *merge(struct directory_files *a, struct directory_files *b, bool by_name, bool desc)
{
    struct directory_files *from = NULL;
    struct directory_files *to = NULL;
    if (!a)
    {
        return b;
    }
    if (!b)
    {
        return a;
    }
    // organiza
    if (by_name)
    {
        if (strcmp(a->filename, b->filename) >= 0)
        {
            from = b;
            to = a;
        }
        else
        {
            from = a;
            to = b;
        }
    }
    else
    {
        if (a->size >= b->size)
        {
            from = b;
            to = a;   
        }
        else
        {
            from = a;
            to = b;
        }
    }

    if(desc)
    {
        struct directory_files *temp = from;
        from = to;
        to = temp;
    }

    from->next = merge(from->next, to, by_name, desc);
    return from;
}

struct directory_files *new_directory_file(char *filename, size_t size)
{
    struct directory_files *ndf = malloc(sizeof(struct directory_files));
    if (ndf)
    {
        strncpy(ndf->filename, filename, NAME_MAX);
        ndf->size = size;
        ndf->next = NULL;
    }

    return ndf;
}

void append_to_root(struct directory_files **root, struct directory_files *new_node)
{
    struct directory_files *last = *root;
    if (*root == NULL)
    {
        *root = new_node;
        return;
    }

    while (last->next != NULL)
    {
        last = last->next;
    }

    last->next = new_node;
    return;
}

void free_directory_files(struct directory_files **root)
{
    struct directory_files *current = *root;
    struct directory_files *next = NULL;
    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }
    *root = NULL;
}

void print_directory_files(struct directory_files *root)
{
    struct directory_files *current = root;
    while (current != NULL)
    {
        printf("filename %s size %ld\n", current->filename, current->size);
        current = current->next;
    }
}

struct directory_files *get_all_files_from_directory(char *dirpath, struct status* s, bool by_name, bool desc)
{
    DIR *directory = NULL;
    struct dirent *directory_entry = NULL;
    struct stat file_stat = {0};
    struct directory_files *root = NULL;

    if ((directory = opendir(dirpath)) == NULL)
    {
        indicate_error(s, "não foi possível abrir essa pasta");
        return NULL;
    }

    int dfd = dirfd(directory);
    do
    {
        errno = 0;
        if ((directory_entry = readdir(directory)) != NULL)
        {

            if (strcmp(directory_entry->d_name, ".") == 0 ||
                strcmp(directory_entry->d_name, "..") == 0)
            {
                continue;
            }

            if (fstatat(dfd, directory_entry->d_name, &file_stat, 0) == -1)
            {
                indicate_error(s, "não foi possível pegar as estatisticas do arquivo");
                return NULL;
            }

            if (S_ISDIR(file_stat.st_mode))
            {
                continue;
            }

            struct directory_files *new_file = new_directory_file(directory_entry->d_name, file_stat.st_size);
            append_to_root(&root, new_file);
        }
    } while (directory_entry != NULL);
    closedir(directory);

    if (errno != 0)
    {
        indicate_error(s, "erro lendo a pasta");
        return NULL;
    }

    // significa que não temos arquivos nessa pasta
    if (root == NULL)
    {
        return root;
    }

    sort_directory_files(&root, by_name, desc);

    return root;
}