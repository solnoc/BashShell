#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DEFAULT_LINES 10
#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    int opt, lines = DEFAULT_LINES, bytes = 0, quiet = 0, always_print_filename = 0;
    char *filename;

    while ((opt = getopt(argc, argv, "n:c:qv")) != -1)
    {    
        switch (opt)
        {
            case 'n':
                lines = atoi(optarg);
                break;
            case 'c':
                bytes = atoi(optarg);
                break;
            case 'q':
                quiet = 1;
                break;
            case 'v':
                always_print_filename = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-n LINES] [-c BYTES] [-q] [-v] FILE\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Set the input stream to read from a file if one is specified, otherwise use stdin
    FILE *input_stream;
    if (optind < argc) {
        filename = argv[optind];

        input_stream = fopen(filename, "r");
        if (input_stream == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
    } else {
        input_stream = stdin;
        filename = "standard input";
    }

    char *line = NULL;
    size_t line_size = 0;

    if (always_print_filename || !quiet)
        printf("==> %s <==\n", filename);

    while (getline(&line, &line_size ,input_stream) != -1)
    {
        if(bytes == 0)
        {
            printf("%s", line);
            if (--lines == 0)
                break;

            continue;
        }

        if(bytes < strlen(line))
        {
            write(1, line, bytes);
        }
        else
        {
            write(1, line, strlen(line));
        }

        bytes -= strlen(line);
        if(bytes <= 0)
            break;
    }

    if (line) {
        free(line);
    }

    if (ferror(input_stream))
    {
        perror("getline");
        exit(EXIT_FAILURE);
    }

    if (input_stream != stdin) {
        fclose(input_stream);
    }

    return 0;
}
