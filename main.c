#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


int main(int argc, char** argv)
{
    int fd[2];
    int fd2[2];
    int id;
    int length;

    if(argc != 3)
    {
        perror("Ussage <exec> <string1> <string2>\n");
        exit(1);
    }

    if(pipe(fd) < 0)
    {
        perror("Error creating pipe\n");
        exit(2);
    }
    
    length = strlen(argv[1]);
    if(pipe(fd2) < 0)
    {
        perror("Error creating pipe\n");
        exit(6);
    }

    if((id=fork()) < 0)
    {
        perror("Error creating fork\n");
        exit(3);    
    }

    if(id == 0)
    {
        close(fd[0]);
        if(write(fd[1],argv[1],length) < 0)
        {
            perror("Error writing to pipe from child\n");
            exit(4);
        }
        printf("Wrote %s to pipe\n", argv[1]);
        close(fd[1]);
    }
    else
    {
        char* buffer = (char*)malloc(sizeof(char) * length);
        close(fd[1]);
        if(read(fd[0],buffer,length) < 0)
        {
            perror("Error reading from pipe from parent\n");
            exit(5);
        }
        printf("Read %s to pipe\n", buffer);
        close(fd[0]);
        free(buffer);
    }
    int length2 = strlen(argv[2]);
    char* buffer = (char*)malloc(sizeof(char)*(length+length2));
    if(id == 0)
    {
        close(fd2[1]);
        if(read(fd2[0],buffer,(length+length2)) < 0)
        {
            perror("Error reading from pipe from child\n");
            exit(7);
        }
        printf("Read %s from pipe\n", buffer);
        close(fd2[0]);
        free(buffer);
    }
    else
    {
        close(fd2[0]);
        char* writingBuffer = (char*)malloc(length + length2);
        strncpy(writingBuffer, argv[1],length);
        strncpy(writingBuffer+length, argv[2],length2);
        if(write(fd2[1],writingBuffer,length+length2) < 0)
        {
            perror("Error writing to pipe from parent\n");
            exit(4);
        }
        printf("Wrote %s to pipe\n", writingBuffer);
        close(fd2[1]);
        free(writingBuffer);
    }
}