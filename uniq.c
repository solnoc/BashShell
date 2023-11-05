#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define BUFFER_SIZE 1

int content;
char buffer[BUFFER_SIZE];
char* previous_line;
char* current_line;
int i = 0;

int compare(char* s1, char* s2, int iflag)
{
    if (iflag == 1)
    {
        return strcasecmp(s1, s2);
    }
    else
    {
        return strcmp(s1, s2);
    }
}

void print_without_repeated_lines(int file_descriptor, int iflag)
{
    int times=0;
    while((content = read(file_descriptor, buffer, BUFFER_SIZE)) > 0)
    {
        if(*buffer == '\n')
        {
            current_line[i] = '\0';
            if (compare(previous_line, current_line, iflag) != 0)
            {
                strncpy(previous_line, current_line, strlen(current_line));
                previous_line[strlen(current_line)] = '\0';

                times = 1;
            }
            else
            {
                times++;
                if (times == 2)
                {
                    printf("%s\n", previous_line);
                }
            }
            i = 0;
        }
        else
        {
            current_line[i] = *buffer;
            i++;
        }
    }
    free(previous_line);
    free(current_line);
}

void print_non_unique_lines(int file_descriptor, int iflag)
{
    while((content = read(file_descriptor, buffer, BUFFER_SIZE)) > 0)
    {
        if(*buffer == '\n')
        {
            current_line[i] = '\0';
            break;
        }
        else
        {
            current_line[i] = *buffer;
            i++;
        }
    }
    printf("%s\n", current_line);
    strncpy(previous_line, current_line, strlen(current_line));
    previous_line[strlen(current_line)] = '\0';
    i = 0;

    while((content = read(file_descriptor, buffer, BUFFER_SIZE)) > 0)
    {
        if(*buffer == '\n')
        {
            current_line[i] = '\0';
            if (compare(previous_line, current_line, iflag) != 0)
            {
                printf("%s\n", current_line);
                strncpy(previous_line, current_line, strlen(current_line));
                previous_line[strlen(current_line)] = '\0';
            }
            i = 0;
        }
        else
        {
            current_line[i] = *buffer;
            i++;
        }
    }
    free(previous_line);
    free(current_line);
}

void print_unique_lines(int file_descriptor, int iflag)
{
    int ok = 0;
    previous_line[0] = '\0';
    while((content = read(file_descriptor, buffer, BUFFER_SIZE)) > 0)
    {
        if(*buffer == '\n')
        {
            current_line[i] = '\0';
            if (compare(previous_line, current_line, iflag) != 0)
            {
                if(ok == 0)
                {
                    printf("%s\n", previous_line);
                }
                strncpy(previous_line, current_line, strlen(current_line));
                previous_line[strlen(current_line)] = '\0';

                ok = 0;
            }
            else
            {
                ok = 1;
            }
            i = 0;
        }
        else
        {
            current_line[i] = *buffer;
            i++;
        }
    }
    if(ok == 0)
    {
        printf("%s\n", previous_line);
    }
    free(previous_line);
    free(current_line);
}


int main(int argc, char **argv)
{
    int file_descriptor, content;
    char buffer[BUFFER_SIZE];
    int c;
    int iflag = 0;
    int uflag = 0;
    int dflag = 0;

    opterr = 0;


    while((c = getopt(argc, argv, "idu")) != -1)
    {
        switch(c)
        {
            case 'i':
                iflag = 1;
                break;
            case 'd':
                dflag = 1;
                break;
            case 'u':
                uflag = 1;
                break;
            case '?':
                break;
            default:
                break;
        }
    }

    previous_line = (char*)malloc(sizeof(char) * 100);
    current_line = (char*)malloc(sizeof(char) * 100);

    if(optind >= argc)
    {
        if (dflag == 1)
        {  
            print_without_repeated_lines(0, iflag);
        }
        else if (uflag == 1)
        {
            print_unique_lines(0, iflag);
        }
        else
        {
            print_non_unique_lines(0, iflag);
        }
    }

    for(int i = optind; i < argc; i++)
    {
        if((file_descriptor = open(argv[i], O_RDONLY)) < 0)
        {
            printf("Error opening the file..\n");
            return 1;
        }

        if (dflag == 1)
        {  
            print_without_repeated_lines(file_descriptor, iflag);
        }

        else if (uflag == 1)
        {
            print_unique_lines(file_descriptor, iflag);
        }
        else
        {
            print_non_unique_lines(file_descriptor, iflag);
        }

        if(close(file_descriptor) < 0)
        {
            printf("Error closing the file..\n");
            return 2;
        }
    }

    return 0;
}