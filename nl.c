#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  // Set default values for the separator string and starting line number
  char *sep = "\t";
  char *delim = "/#";

  // Parse command-line options
  int opt;
  while ((opt = getopt(argc, argv, "s:d:")) != -1) {
    switch (opt) {
      case 's':
        sep = optarg;
        break;
      case 'd':
        delim = optarg;
        break;
      default:
        fprintf(stderr, "Usage: %s [-s sep] [-d delim] [file...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  // Set the input stream to read from a file if one is specified, otherwise use stdin
  FILE *input_stream;
  if (optind < argc) {
    input_stream = fopen(argv[optind], "r");
    if (input_stream == NULL) {
      perror("fopen");
      exit(EXIT_FAILURE);
    }
  } else {
    input_stream = stdin;
  }

  // Read and process the input line by line
  bool inBody = true;
  char *line = NULL;
  size_t line_size = 0;
  int line_number = 1;
  
  char* delime3 = malloc(strlen(delim) * 3 + 2);
  memccpy(delime3, delim, strlen(delim), strlen(delim));
  memccpy(delime3 + strlen(delim), delim, strlen(delim), strlen(delim));
  memccpy(delime3 + strlen(delim) * 2, delim, strlen(delim), strlen(delim));
  delime3[strlen(delim) * 3] = '\n';
  delime3[strlen(delim) * 3 + 1] = '\0';

  char* delime2 = malloc(strlen(delim) * 2 + 2);
  memccpy(delime2, delim, strlen(delim), strlen(delim));
  memccpy(delime2 + strlen(delim), delim, strlen(delim), strlen(delim));
  delime2[strlen(delim) * 2] = '\n';
  delime2[strlen(delim) * 2 + 1] = '\0';

  
  char* delim_copy = malloc(strlen(delim) + 2);
  memccpy(delim_copy, delim, strlen(delim), strlen(delim));
  delim_copy[strlen(delim)] = '\n';
  delim_copy[strlen(delim) + 1] = '\0';

  delim = delim_copy;

  while (getline(&line, &line_size, input_stream) != -1) {

    if (line[0] == '\n') {
      printf("\n");
      continue;
    }

    if(!strncmp(delim, line, strlen(delim)) || !strncmp(delime3, line, strlen(delime3)))
    {
        inBody = false;
        line_number = 1;
        continue;
    }
    else if(!strncmp(delime2,line,  strlen(delime2)))
    {
        inBody = true;
        continue;
    }
        
        
    // Print the line number, delimiter string, and line text
    if (inBody) {
      printf("%d%s%s", line_number, sep, line);
      line_number++;
    }
    else
    {
        printf("%s", line);
    }
  }
  printf("\n");

  free(delim_copy);
  free(delime3);
  free(delime2);

  // Clean up
  free(line);
  if (input_stream != stdin) {
    fclose(input_stream);
  }
  exit(EXIT_SUCCESS);
}
