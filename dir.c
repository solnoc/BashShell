#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

// Exit codes:
// 1 - memory allocation error
// 2 - error opening directory
// 3 - error closing directory

int find_nr_files(const char* path, char opt)
{
	DIR* directory = opendir(path);
	if(directory == NULL)
	{
        write(2, "Error opening directory for \"dir\" command.\n", 43);
		exit(2);
	}

	int files = 0;
	struct dirent* current_file_iterator;
	while((current_file_iterator = readdir(directory)) != NULL)
	{
		if (current_file_iterator->d_name[0] != '.')
		{
			files++;
		}
	}
	if(closedir(directory) < 0)
	{
        write(2, "Error closing directory for \"dir\" command.\n", 43);
		exit(3);
	}

	return files;
}
void sort_files(char** files, int nr_files)
{
    int i, j;
    for(i = 0; i < nr_files - 1; i++)
    {
        for(j = i + 1; j < nr_files; j++)
        {
            if(strcmp(files[i], files[j]) > 0)
            {
                char* temp = files[i];
                files[i] = files[j];
                files[j] = temp;
            }
        }
    }
}
void print_files(char** files, int nr_files)
{
    int i;
    for(i = 0; i < nr_files; i++)
    {
        write(1, files[i], strlen(files[i]));
        write(1, "  ", 2);
    }
    write(1, "\n", 1);
}
void dir(char* path)
{
    int nr_files = find_nr_files(path, 0);
    char** files = malloc(nr_files * sizeof(char*));
    if(files == NULL)
    {
        write(2, "Error allocating memory for \"dir\" command.\n", 43);
        exit(1);
    }

    DIR* directory = opendir(path);
    if(directory == NULL)
    {
        write(2, "Error opening directory for \"dir\" command.\n", 43);
        exit(2);
    }

    int i = 0;
    struct dirent* current_file_iterator;
    while((current_file_iterator = readdir(directory)) != NULL)
    {
        if(current_file_iterator->d_name[0] != '.')
        {
            files[i] = current_file_iterator->d_name;
            i++;
        }
    }

    sort_files(files, nr_files);
    print_files(files, nr_files);

    if(closedir(directory) < 0)
    {
        write(2, "Error closing directory for \"dir\" command.\n", 43);
        exit(3);
    }

    free(files);
}

int main(int argc, char* argv[])
{
    if(argc == 1)
    {
        dir(".");
    }
    else if(argc == 2)
    {
        dir(argv[1]);
    }
    else
    {
        for(int i = 1; i < argc; i++)
        {
            write(1, argv[i], strlen(argv[i]));
            write(1, ":\n", 2);
            dir(argv[i]);
            write(1, "\n", 1);
        }
    }

    return 0;

}