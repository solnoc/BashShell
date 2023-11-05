#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

struct folder
{
    string name;
    folder *next;
    folder *prev;
};
class Path
{
public:

    folder *head;
    folder *tail;

    Path(string path)
    {
        head = new folder;
        head->name = "";
        head->next = NULL;
        head->prev = NULL;
        tail = head;

        int i = 1;
        while (i < path.length())
        {
            string folder_name = "";
            while (path[i] != '/' && i < path.length())
            {
                folder_name += path[i];
                i++;
            }
            i++;
            folder *new_folder = new folder;
            new_folder->name = folder_name;
            new_folder->next = NULL;
            new_folder->prev = tail;
            tail->next = new_folder;
            tail = new_folder;
        }
    }

    string get_path()
    {
        string path = "";
        folder *temp = head;
        path += temp->name;
        temp = temp->next;
        while (temp != NULL)
        {
            path += "/";
            path += temp->name;
            temp = temp->next;
        }
        return path;
    }

    void cd(string folder_name)
    {
        if (folder_name == "..")
        {
            if (tail->prev != NULL)
            {
                tail = tail->prev;
                tail->next = NULL;
            }
        }
        else
        {
            folder *new_folder = new folder;
            new_folder->name = folder_name;
            new_folder->next = NULL;
            new_folder->prev = tail;
            tail->next = new_folder;
            tail = new_folder;
        }
    }
};
struct command
{
    string command;
    char** arguments;
    int argument_count;
};
command get_command_from_string(string input_command)
{
    command output_command;

    output_command.argument_count = 0;
    int i = 0;
    while(i < input_command.length())
    {
        if(input_command[i] == ' ')
        {
            output_command.argument_count++;
            while (input_command[i] == ' ')
            {
                i++;
            }
        }
        i++;
    }

    output_command.argument_count += 2;
    output_command.arguments = new char*[output_command.argument_count];
    i=0;
    while (input_command[i] == ' ')
    {
        i++;
    }
    int command_start = i;
    while (input_command[i] != ' ' && input_command[i] != '\0')
    {
        i++;
    }
    int command_end = i;
    output_command.command = input_command.substr(command_start, command_end - command_start);

    output_command.arguments[0] = new char[output_command.command.length() + 1];
    strncpy(output_command.arguments[0], output_command.command.c_str(), output_command.command.length() + 1);
    output_command.arguments[0][output_command.command.length()] = '\0';

    int argc = 1;
    while (i < input_command.length())
    {
        while (input_command[i] == ' ')
        {
            i++;
        }
        int argument_start = i;
        while (input_command[i] != ' ' && input_command[i] != '\0')
        {
            i++;
        }
        int argument_end = i;

        string argument = input_command.substr(argument_start, argument_end - argument_start);
        output_command.arguments[argc] = new char[argument.length() + 1];
        strncpy(output_command.arguments[argc], argument.c_str(), argument.length() + 1);
        output_command.arguments[argc][argument.length()] = '\0';

        argc++;
    }
    output_command.arguments[argc] = NULL;

    return output_command;
}

command* get_command_list(string input_command, int &command_count)
{
    int i = 0;
    command_count = 0;
    command *output_command = new command[100];
    while (i < input_command.length())
    {
        while (input_command[i] == ' ')
        {
            i++;
        }
        int command_start = i;
        while (input_command[i] != '|' &&
                input_command[i] != '<' &&
                input_command[i] != '>' &&
                input_command[i] != '\n' &&
                input_command[i] != '\0')
        {
            i++;
        }
        int command_end = i;

        output_command[command_count] = get_command_from_string(input_command.substr(command_start, command_end - command_start + 1));
        
        command_count++;
        i++;
    }

    return output_command;
}
bool check_file_existance(string file_name)
{
    if (access(file_name.c_str(), F_OK) != -1)
    {
        return true;
    }
    else
    {
        return false;
    }
}
void bind_redirects(command* command_list, int command_count)
{
    int i = 0;
    while(command_list[i].arguments[command_list[i].argument_count - 2][0] == '|' || 
            command_list[i].arguments[command_list[i].argument_count - 2][0] == '<' || 
            command_list[i].arguments[command_list[i].argument_count - 2][0] == '>')
    {
        i++;
        if(command_list[i-1].arguments[command_list[i-1].argument_count - 2][0] == '<')
        {
            if(check_file_existance(command_list[i].command))
            {
                int fd;
                if((fd = open(command_list[i].command.c_str(), O_RDONLY)) >= -1)
                {
                    dup2(fd, 0);
                }
                else
                {
                    printf("Error opening file.\n");
                    exit(0);
                }
            }
            else
            {
                printf("File does not exist.\n");
                exit(0);
            }
        }
        else if(command_list[i-1].arguments[command_list[i-1].argument_count - 2][0] == '>')
        {
            int fd;
            if((fd = open(command_list[i].command.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644)) >= 0)
            {
                dup2(fd, 1);
            }
            else
            {
                printf("Error opening file.\n");
                exit(0);
            }
        }
    }
}


void execute_command(command commanda)
{
    if(commanda.arguments[commanda.argument_count - 2][0] == '|' || 
            commanda.arguments[commanda.argument_count - 2][0] == '<' || 
            commanda.arguments[commanda.argument_count - 2][0] == '>')
    {
        commanda.arguments[commanda.argument_count - 2] = NULL;
    }
    if(commanda.command == "tail")
    {
        if(execv("/home/solnoc/Faculta/OS1/Help/Aryana/tail", commanda.arguments) == -1)
        {
            printf("Error executing tail.\n");
            exit(0);
        }
    }
    else if(commanda.command == "uniq")
    {
        if(execv("/home/solnoc/Faculta/OS1/Help/Aryana/uniq", commanda.arguments) == -1)
        {
            printf("Error executing uniq.\n");
            exit(0);
        }
    }
    else
    {
        if(execv((((string)"/bin/").append(commanda.command)).c_str(), commanda.arguments) == -1)
        {
            printf("Error executing command.\n");
            exit(0);
        }
    }
}

void print_prompt(Path path)
{
    cout << path.get_path() << " $>";
}
int main()
{
    Path path("/home/solnoc/Faculta/OS1/Help/Aryana");
    
    print_prompt(path);

    string commanda;
    bool run = true;
    
    while(run)
    {
        getline(cin, commanda);

        int command_count;
        command* input_command;
        input_command = get_command_list(commanda, command_count);

        bool piped = false;
        int second_command_index = -1;
        for(int i = 0; i < command_count; i++)
        {
            if(input_command[i].arguments[input_command[i].argument_count - 2][0] == '|')
            {
                piped = true;
                second_command_index = i+1;
                break;
            }
        }

        if(input_command[0].command == "exit")
        {
            run = false;
            break;
        }
        else if(input_command[0].command == "cd")
        {
            if(chdir(input_command[0].arguments[1]) == -1)
            {
                printf("Error executing cd.\n");
                exit(0);
            }
            else
            {
                path.cd(input_command[0].arguments[1]);
            }
        }
        else if(piped)
        {
            int fd[2];
            if(pipe(fd) == -1)
            {
                printf("Error creating pipe.\n");
                exit(0);
            }
            if(fork() == 0)
            {
                bind_redirects(input_command, second_command_index - 1);
                close(fd[0]);
                dup2(fd[1], 1);
                execute_command(input_command[0]);
                exit(0);
            }
            else if(fork() == 0)
            {
                bind_redirects(input_command + second_command_index, command_count - second_command_index);
                close(fd[1]);
                dup2(fd[0], 0);
                execute_command(input_command[second_command_index]);
                exit(0);
            }
            else
            {
                wait(NULL);
            }
        }
        else
        {
            if(fork() == 0)
            {
                bind_redirects(input_command, command_count);
                execute_command(input_command[0]);
                exit(0);
            }
            else
            {
                wait(NULL);
            }
        }

        delete[] input_command;
        fflush(stdout);
        print_prompt(path);
    }

    return 0;
}