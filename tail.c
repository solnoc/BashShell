#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>


#define CHUNK_SIZE 16

void get_last_nr_bytes_from_file(int file_descriptor, char* buffer, int nr_bytes)
{
    char* characters_read = malloc(nr_bytes);
    int filesize = 0;
    filesize = lseek(file_descriptor, 0, SEEK_END);
    int i = filesize - 1;
    int counter = 0;
    while(i >= 0)
    {   
        lseek(file_descriptor, i, SEEK_SET);
        read(file_descriptor, buffer, 1);
        characters_read[counter] = buffer[0];
        i--;
        counter++;
        if(counter == nr_bytes)
        {
            break;
        }
    }
    for(int i = nr_bytes - 1; i >= 0; i--)
    {
        printf("%c", characters_read[i]);
    }

    free(characters_read);
}

void insert_line_position(int n, long* line_positions, long position)
{
    for(int i = n; i > 0; i--)
    {
        line_positions[i] = line_positions[i-1];
    }
    line_positions[0] = position;
}

void get_last_n_lines_positions(int n, char* buffer, int size, int* lines, int chunk_tested, long* line_positions)
{
    int poz = 0;
    for(int i = 0; i < size; i++)
    {
        if(buffer[i] == '\n')
        {
            (*lines)++;
            poz = i + (chunk_tested * CHUNK_SIZE);
            insert_line_position(n, line_positions, poz);
        }
    }
}

void read_last_n_lines(int file_descriptor, char* buffer, int n , long* line_positions)
{
    int content = 0;
    int lines = 0;
    int chunks_tested = 0;
    while ((content = read(file_descriptor, buffer, CHUNK_SIZE)) > 0)
    {
        get_last_n_lines_positions(n, buffer, content, &lines, chunks_tested, line_positions);
        chunks_tested++;
    }

    lseek(file_descriptor, line_positions[n], SEEK_SET);
    
    while((content = read(file_descriptor, buffer, CHUNK_SIZE)) > 0)
    {
        buffer[content] = '\0';
        printf("%s", buffer);
    }
}

int check_open_and_print(char* path, int qflag, int vflag, int argc)
{
    int file_descriptor = 0;
    if((file_descriptor = open(path, O_RDONLY)) < 0)
    {
        printf("Error opening file");
        return 1;
    }

    if (argc - optind > 1 && qflag == 0 && vflag == 0)
    {
        printf("====>: %s<====>\n", path);
    }
    else if (vflag == 1)
    {
        printf("*****File name: %s *****\n", path);
    }
    return file_descriptor;
}

int main(int argc, char** argv)
{

    int n = 10;
    int cflag = 0;
    int nflag = 0;
    int qflag = 0;
    int vflag = 0;
    int bytes = 0;

    int c;
    int opterror = 0;

    while((c = getopt(argc, argv, "c:n:qv")) != -1)
    {
        switch(c)
        {
            case 'c':
                cflag = 1;
                bytes = atoi(optarg);
                break;
            case 'n':
                nflag = 1;
                n = atoi(optarg);
                break;
            case 'q':
                qflag = 1;
                break;
            case 'v':
                vflag = 1;
                break;
            case '?':
                break;
        }
    }

    int file_descriptor = 0;
    int lines = 0;    
    char buffer[CHUNK_SIZE];
    long* line_positions = (long*)malloc((n + 1) * sizeof(long));
    memset(line_positions, 0, (n + 1) * sizeof(long));
    int content = 0;

    if(optind >= argc)
    {
        if (vflag == 1)
        {
            printf("*****standard input*****\n");
        }
        if ((file_descriptor = open("temporary_tail.txt", O_CREAT | O_WRONLY | O_TRUNC, 0666)) < 0)
        {
            printf("Error opening file");
            return 1;
        }
        while ((content = read(0, buffer, CHUNK_SIZE)) > 0)
        {
            write(file_descriptor, buffer, content);
        }
        close(file_descriptor);

        if ( (file_descriptor = open("temporary_tail.txt", O_RDONLY)) < 0)
        {
            printf("Error opening file");
            return 1;
        }

        if(cflag == 1)
        {
            get_last_nr_bytes_from_file(file_descriptor, buffer, bytes);
        }
        else
        {
            read_last_n_lines(file_descriptor, buffer, n, line_positions);
        }
        if (close(file_descriptor) < 0)
        {
            printf("Error closing file");
            return 1;
        }
    }


   for(int i = optind; i < argc; i++)
   {
        file_descriptor = check_open_and_print(argv[i], qflag, vflag, argc);

        if(cflag == 1)
        {
            printf("argv[optind] = %s\n", argv[i]);
            get_last_nr_bytes_from_file(file_descriptor, buffer, bytes);
        }
        else
        {
            read_last_n_lines(file_descriptor, buffer, n, line_positions);
        }
        if (close(file_descriptor) < 0)
        {
            printf("Error closing file");
            return 1;
        }
   }
}