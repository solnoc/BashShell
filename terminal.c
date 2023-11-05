#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>

#define MAX_COMMAND_LENGTH 1024

// Exit codes:
// 1 - memory allocation error
// 2 - error opening directory
// 3 - error closing directory
// 4 - error getting file data
// 5 - argument missing for option error
// 6 - invalid option error
// 7 - error opening file
// 8 - error closing file
// 9 - error getting file size
// 10 - error moving file descriptor position
// 11 - error executing command

struct command_with_args
{
    char* command;
    char** args;
    int nr_args;
    struct command_with_args* next;
    char linking_operator;
};

char* find_next_space_charachter(char* str)
{
    while(*str != ' ' && *str != '\0' && *str != '\n')
    {
        str++;
    }
    return str;
}
char* find_where_command_ends(char* str)
{
    char* it = str;
    while(*it != '|' && *it != '>' && *it != '<' && *it != '?' && *it != '\0' && *it != '\n')
    {
        it++;
    }
    return it;
}
struct command_with_args* give_command_and_args_from_input(char* buffer)
{
    char* command = buffer;
    int buffer_length = strlen(buffer);
            
    struct command_with_args* command_and_args = malloc(sizeof(struct command_with_args));
    struct command_with_args* first_command_and_args = command_and_args;
    while(command - buffer < buffer_length)
    {
        while(*command == ' ')
        {
            command++;
        }
        char* command_end = find_where_command_ends(command);

        char finishing_char = *command_end;
        if(finishing_char == '\n')
        {
            finishing_char = '\0';
        }

        *command_end = '\0';
        while(*(command_end-1) == ' ')
        {
            command_end--;
            *command_end = '\0';
        }

        give_command_and_args(command, &command_and_args);

        command_and_args->linking_operator = finishing_char;

        if(finishing_char != '\0')
        {
            command_and_args->next = malloc(sizeof(struct command_with_args));
            command_and_args = command_and_args->next;
        }

        command_end++;
        command = command_end;
    }
    return first_command_and_args;
}
void give_command_and_args(char* str, struct command_with_args** command_and_args)
{
    (*command_and_args)->nr_args = 2;
    (*command_and_args)->next = NULL;
    (*command_and_args)->linking_operator = '\0';

    char* it = find_next_space_charachter(str);

    (*command_and_args)->command = malloc(it - str + 1);
    memcpy((*command_and_args)->command, str, it - str);
    (*command_and_args)->command[it - str] = '\0';

    while (*it == ' ')
    {
        it++;    
    }
    
    while(*it != '\0' && *it != '\n')
    {
        (*command_and_args)->nr_args++;
        it = find_next_space_charachter(it);
        while(*it == ' ')
        {
            it++;
        }
    }

    (*command_and_args)->args = malloc((*command_and_args)->nr_args * sizeof(char*));

    (*command_and_args)->args[0] = malloc(strlen((*command_and_args)->command) + 1);
    memcpy((*command_and_args)->args[0], (*command_and_args)->command, strlen((*command_and_args)->command));
    (*command_and_args)->args[0][strlen((*command_and_args)->command)] = '\0';

    (*command_and_args)->args[(*command_and_args)->nr_args - 1] = NULL;

    it = find_next_space_charachter(str);
    while(*it == ' ')
    {
        it++;
    }

    for(int i=1;i<(*command_and_args)->nr_args-1;i++)
    {
        char* it_fin = find_next_space_charachter(it);

        (*command_and_args)->args[i] = malloc(it_fin - it + 1);
        memcpy((*command_and_args)->args[i], it, it_fin - it);
        (*command_and_args)->args[i][it_fin - it] = '\0';

        it = it_fin;
        while(*it == ' ')
        {
            it++;
        }
    }
}
void execute_command(char* command, char** args, int nr_args)
{
    printf("Executing command: %s\n", command);
    if(strcmp(command,"ls") == 0 || strcmp(command, "tac") == 0 || strcmp(command, "dir") == 0)
    {
        char* exec_command = malloc(strlen(command) + 3);
        exec_command[0] = '.';
        exec_command[1] = '/';
        memcpy(exec_command + 2, command, strlen(command));
        exec_command[strlen(command) + 2] = '\0';

        if(execv(exec_command, args) < 0)
        {
            printf("Error executing command: %s\n", command);
            exit(11);
        }
    }
    else
    {
        char* exec_command = malloc(strlen(command) + 5);
        strncpy(exec_command, "/bin/", 5);
        memcpy(exec_command + 5, command, strlen(command));
        exec_command[strlen(command) + 5] = '\0';

        if(execv(exec_command, args) < 0)
        {
            printf("Error executing command: %s\n", command);
            exit(11);
        }
    }
}
void set_output_of_command_to_file(char* file_name)
{
    int fd;
    if((fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
    {
        printf("Error opening file: %s\n", file_name);
        exit(7);
    }
    dup2(fd, 1);
    close(fd);
}
void set_input_of_command_from_file(char* file_name)
{
    int fd;
    if((fd = open(file_name, O_RDONLY)) < 0)
    {
        printf("Error opening file: %s\n", file_name);
        exit(7);
    }
    dup2(fd, 0);
    close(fd);
}
void set_error_of_command_to_file(char* file_name)
{
    int fd;
    if((fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
    {
        printf("Error opening file: %s\n", file_name);
        exit(7);
    }
    dup2(fd, 2);
    close(fd);
}
struct command_with_args* set_up_command(struct command_with_args* command_with_args)
{
    while(command_with_args->linking_operator != '|' && command_with_args->linking_operator != '\0' && command_with_args->linking_operator != '\n')
    {
        if(command_with_args->linking_operator == '>')
        {
            set_output_of_command_to_file(command_with_args->next->command);
        }
        else if(command_with_args->linking_operator == '<')
        {
            set_input_of_command_from_file(command_with_args->next->command);
        }
        else if(command_with_args->linking_operator == '?')
        {
            set_error_of_command_to_file(command_with_args->next->command);
        }
        command_with_args = command_with_args->next;
    }
    return command_with_args;
}

int run_command_and_args(struct command_with_args* command_with_args, int* pipefd)
{
    struct command_with_args* current_command_with_args = command_with_args;

    if(pipefd != NULL)
    {
        dup2(pipefd[0], 0);
        close(pipefd[1]);
    }

    command_with_args = set_up_command(command_with_args);
    if(command_with_args->linking_operator == '|')
    {
        int pipefd[2];
        int pid;
        pipe(pipefd);
        if((pid = fork()) == 0)
        {
            run_command_and_args(command_with_args->next, pipefd);
        }
        else
        {
            close(pipefd[0]);
            dup2(pipefd[1], 1);

            execute_command(current_command_with_args->command, current_command_with_args->args, current_command_with_args->nr_args);
            wait(NULL);
        }
        
    }
    else
    {
        execute_command(current_command_with_args->command, current_command_with_args->args, current_command_with_args->nr_args);
    }

    return 0;
}

int main(int argc, char** argv)
{
    char buffer[MAX_COMMAND_LENGTH + 2];
    memset(buffer, '\0', MAX_COMMAND_LENGTH + 1);
    buffer[MAX_COMMAND_LENGTH + 1] = '\0';

    write(1, "Welcome to the shell!\n", 22);
    write(1, "$> ", 3);
    while(read(0, buffer, MAX_COMMAND_LENGTH + 1) > 0)
    {
        if(strlen(buffer) > MAX_COMMAND_LENGTH)
        {
            write(2, "Error: command too long.\n", 25);
            continue;
        }
            
        struct command_with_args* command_and_args = give_command_and_args_from_input(buffer);

        if(strcmp(command_and_args->command, "exit") == 0)
        {
            exit(0);
        }

        int pid;
        if((pid = fork()) == 0)
        {
            run_command_and_args(command_and_args, (void*)0);
        }
        else
        {
            wait(NULL);
        }

        memset(buffer, '\0', MAX_COMMAND_LENGTH + 1);
        while(command_and_args != NULL)
        {
            struct command_with_args* temp = command_and_args;
            command_and_args = command_and_args->next;
            free(temp);
        }

        write(1, "$> ", 3);
    }
    
    return 0;
}