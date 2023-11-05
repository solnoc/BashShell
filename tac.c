#include "stdio.h"
#include "getopt.h"
#include "fcntl.h"
#include "unistd.h"
#include "stdlib.h"
#include "string.h"
#include "signal.h"

#define _b 0b0001
#define _s 0b0010

#define CHUNK_SIZE 8

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

int stop_input = 0;
void handle_signal(int signal)
{
    if (signal == SIGINT) {
        stop_input = 1;
        write(1, "Stopped reading input.\n", 23);
    }
}


long get_file_size(int fd)
{
    long size;
    if((size = lseek(fd, 0, SEEK_END)) < 0)
    {
        write(2, "Error getting file size for \"tac\" command.\n", 43);
        exit(9);
    }
    if(lseek(fd, 0, SEEK_SET) < 0)
    {
        write(2, "Error moving file descriptor position for \"tac\" command.\n", 57);
        exit(10);
    }

    return size;
}
long get_nr_of_separator(int fd, char separator)
{
    char* buffer = malloc(CHUNK_SIZE);
    if(buffer == NULL)
    {
        write(2, "Memory allocation error for \"tac\" command.\n", 44);
        exit(1);
    }

    int nr_of_separator = 0;
    while(read(fd, buffer, CHUNK_SIZE) > 0)
    {
        int i;
        for(i = 0; i < CHUNK_SIZE; i++)
        {
            if(buffer[i] == separator)
            {
                nr_of_separator++;
            }
        }
    }
    if(lseek(fd, 0, SEEK_SET) < 0)
    {
        write(2, "Error moving file descriptor position for \"tac\" command.\n", 57);
        exit(10);
    }

    free(buffer);
    return nr_of_separator + 1;
}
int* get_positions_of_separators(int fd, char separator, int nr_of_separator, char opt)
{
    char* buffer = malloc(CHUNK_SIZE);
    if(buffer == NULL)
    {
        write(2, "Memory allocation error for \"tac\" command.\n", 44);
        exit(1);
    }

    int* positions = malloc(sizeof(int) * nr_of_separator);
    if(positions == NULL)
    {
        write(2, "Memory allocation error for \"tac\" command.\n", 44);
        exit(1);
    }

    int chunks = 0;
    int i;
    int j = 1;
    positions[0] = 0;

    while(read(fd, buffer, CHUNK_SIZE) > 0)
    {
        for(i = 0; i < CHUNK_SIZE; i++)
        {
            if(buffer[i] == separator)
            {
                positions[j] = i + chunks * CHUNK_SIZE + (opt & _b ? 0 : 1);
                j++;
            }
        }
        chunks++;
    }
    if(lseek(fd, 0, SEEK_SET) < 0)
    {
        write(2, "Error moving file descriptor position for \"tac\" command.\n", 57);
        exit(10);
    }

    free(buffer);
    return positions;
}
void write_from_position_to_next(int fd, int i, int* positions, int nr_of_positions)
{
    if(lseek(fd, positions[i], SEEK_SET) < 0)
    {
        write(2, "Error moving file descriptor position for \"tac\" command.\n", 57);
        exit(10);
    }

    int j;
    if(i == nr_of_positions - 1)
    {
        j = get_file_size(fd);
    }
    else
    {
        j = positions[i + 1];
    }

    int chunks = 1;
    char* buffer = malloc(CHUNK_SIZE + 1);
    if(buffer == NULL)
    {
        write(2, "Memory allocation error for \"tac\" command.\n", 44);
        exit(1);
    }

    int n;
    while((n = read(fd, buffer, CHUNK_SIZE)) > 0)
    {
        if(n < 0)
        {
            write(2, "Error getting file data for \"tac\" command.\n", 43);
            exit(4);
        }
        else if(positions[i] + chunks * CHUNK_SIZE >= j)
        {
            write(1, buffer, j - (positions[i] + (chunks - 1) * CHUNK_SIZE));
            break;
        }
        else if(n < CHUNK_SIZE)
        {
            write(1, buffer, n);
            break;
        }
        chunks++;
        write(1, buffer, CHUNK_SIZE);
    }
    free(buffer);
}
void write_file_data(char* path, char opt, char separator)
{
    int fd;

    if(path == NULL)
    {
        if((fd = open("/tmp/tac_temp_file.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0)
        {
            write(2, "Error opening file for \"tac\" command.\n", 39);
            exit(7);
        }

        char* buffer = malloc(CHUNK_SIZE);
        if(buffer == NULL)
        {
            write(2, "Memory allocation error for \"tac\" command.\n", 44);
            exit(1);
        }

        int n;
        while((n = read(0, buffer, CHUNK_SIZE)) > 0)
        {
            if(n < 0)
            {
                write(2, "Error getting file data for \"tac\" command.\n", 43);
                exit(4);
            }
            write(fd, buffer, n);
        }
        
        free(buffer);
        if(close(fd) < 0)
        {
            write(2, "Error closing file for \"tac\" command.\n", 39);
            exit(8);
        }

        if((fd = open("/tmp/tac_temp_file.txt", O_RDONLY)) < 0)
        {
            write(2, "Error opening file for \"tac\" command.\n", 39);
            exit(7);
        }

    }
    else if((fd = open(path, O_RDONLY)) < 0)
    {
        write(2, "Error opening file for \"tac\" command.\n", 39);
        exit(7);
    }

    int nr_of_separator = get_nr_of_separator(fd, separator);
    int* positions = get_positions_of_separators(fd, separator, nr_of_separator, opt);

    char* buffer = malloc(CHUNK_SIZE);
    if(buffer == NULL)
    {
        write(2, "Memory allocation error for \"tac\" command.\n", 44);
        exit(1);
    }

    for(int i=nr_of_separator - 1;i >= 0;i--)
    {
        write_from_position_to_next(fd, i, positions, nr_of_separator);
    }

    if(close(fd) < 0)
    {
        write(2, "Error closing file for \"tac\" command.\n", 39);
        exit(8);
    }

}
int main(int argc, char** argv)
{
    signal(SIGINT, handle_signal);

    char opt = 0;
    char separator = '\n';

    opterr = 0;
    int option;
    while((option = getopt(argc, argv, "bs:")) != -1)
    {
        switch(option)
        {
            case 'b':
                opt |= _b;
                break;
            case 's':
                opt |= _s;
                separator = optarg[0];
                break;

            case ':':
               if(optopt == 's')
               {
                   write(2, "Argument missing for option \"-s\".\n", 34);
                   exit(5);
               }
               break;
            default:
                write(2, "Invalid option.\n", 16);
                exit(6);
        }
    }

    if(optind == argc)
    {
        write_file_data((void*)0, opt, separator);
    }

    for(int i=optind; i < argc; i++ )
    {
        write_file_data(argv[i], opt, separator);
    }

    return 0;
}