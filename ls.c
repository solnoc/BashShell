#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>

#define _l       0b0001
#define _s       0b0010
#define _a       0b0100
#define _F       0b1000
#define _ofd 0b00010000
#define _af  0b00100000

#define BLUE  "\033[0;34m"
#define GREEN "\033[0;32m"
#define STOP_COLOR "\033[0m"

#define COLOR_SIZE 7
#define STOP_COLOR_SIZE 4

// Exit codes:
// 1 - memory allocation error
// 2 - error opening directory
// 3 - error closing directory
// 4 - error getting file data
// 5 - argument missing for option error
// 6 - invalid option error

struct file_data
{
	struct dirent* file_name_and_type;
	struct stat* file_data;
	char* rich_name;
};

void sort_file(struct file_data* files, int nr_files)
{
	struct file_data* buf = malloc(sizeof(struct file_data));
	if(buf == NULL)
	{
		write(2, "Error allocating memory for \"ls\" command.\n", 43);
		exit(1);
	}

	memset(buf, 0, sizeof(struct file_data));
	for(int i=1;i<nr_files;i++)
	{
		int j = i;
		while (j > 0 && strcmp(files[j].file_name_and_type->d_name, files[j-1].file_name_and_type->d_name) < 0)
		{
			memcpy(buf, &files[j],sizeof(struct file_data));
			files[j] = files[j-1];
			files[j-1] = *buf;
			j--;
		}
	}
	free(buf);
}

char* int_to_string(int nr)
{
	if(nr == 0)
	{
		char* buf = malloc(2);
		if(buf == NULL)
		{
			write(2, "Error allocating memory for \"ls\" command.\n", 43);
			exit(1);
		}

		buf[0] = '0';
		buf[1] = '\0';
		return buf;
	}

	int nr_digits = 0;
	int tmp = nr;
	while(tmp)
	{
		nr_digits++;
		tmp /= 10;
	}

	char* ret = malloc(nr_digits + 1);
	if(ret == NULL)
	{
		write(2, "Error allocating memory for \"ls\" command.\n", 43);
		exit(1);
	}

	ret[nr_digits] = '\0';
	while(nr_digits)
	{
		nr_digits--;
		ret[nr_digits] = nr % 10 + '0';
		nr /= 10;
	}
	return ret;
}
char* add_path(const char* filepath, const char* filename)
{
	size_t filepath_size = strlen(filepath);
	size_t filename_size = strlen(filename);

	char* ret = malloc(filepath_size + filename_size + 2);
	if(ret == NULL)
	{
		write(2, "Error allocating memory for \"ls\" command.\n", 43);
		exit(1);
	}

	memcpy(ret, filepath, filepath_size);
	
	ret[filepath_size] = '/';
	memcpy(ret + filepath_size + 1, filename, filename_size);
	
	ret[filepath_size + filename_size + 1] = '\0';
	return ret;
}
char* add_color(const char* str, const char* color)
{
	size_t string_size = strlen(str);
	char* begin = malloc(COLOR_SIZE + STOP_COLOR_SIZE + string_size + 1);
	if(begin == NULL)
	{
		write(2, "Error allocating memory for \"ls\" command.\n", 43);
		exit(1);
	}

	char* end = begin;

	memcpy(end, color, COLOR_SIZE);
	end += COLOR_SIZE;

	memcpy(end, str, string_size);
	end += string_size;

	memcpy(end, STOP_COLOR, STOP_COLOR_SIZE);
	end += STOP_COLOR_SIZE;
	*end = '\0';

	return begin;
}
char* append_charachter(const char* str, char chr)
{
	size_t buffer_size = strlen(str);
	char* ret = malloc(strlen(str) + 2);
	if(ret == NULL)
	{
		write(2, "Error allocating memory for \"ls\" command.\n", 43);
		exit(1);
	}

	memcpy(ret, str, buffer_size);
	ret[buffer_size] = chr;
	ret[buffer_size + 1] = '\0';
	return ret;
}
void enrich_file_name(struct file_data* file, char opt)
{
	char* buf;
	size_t buffer_size;
	
	if(file->file_name_and_type->d_type == DT_DIR)
	{
		if(opt & _F)
		{
			if(!(opt & _ofd))
			{
				buf = add_color(file->file_name_and_type->d_name, BLUE);
				file->rich_name = append_charachter(buf, '/');
				free(buf);
			}
			else
			{
				file->rich_name = append_charachter(file->file_name_and_type->d_name, '/');
			}
		}
		else
		{
			if(!(opt & _ofd))
			{
				file->rich_name = add_color(file->file_name_and_type->d_name, BLUE);
			}
			else
			{
				file->rich_name = malloc(strlen(file->file_name_and_type->d_name) + 1);
				memcpy(file->rich_name, file->file_name_and_type->d_name, strlen(file->file_name_and_type->d_name) + 1);
			}
		}
	}
	else if(file->file_data->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
	{
		if(opt & _F)
		{
			if(!(opt & _ofd))
			{
				buf = add_color(file->file_name_and_type->d_name, GREEN);
			}
			else
			{
				buf = malloc(strlen(file->file_name_and_type->d_name) + 1);
				memcpy(buf, file->file_name_and_type->d_name, strlen(file->file_name_and_type->d_name) + 1);
			}
			file->rich_name = append_charachter(buf, '*');
			free(buf);
		}
		else
		{
			if(!(opt & _ofd))
			{
				file->rich_name = add_color(file->file_name_and_type->d_name, GREEN);
			}
			else
			{
				file->rich_name = malloc(strlen(file->file_name_and_type->d_name) + 1);
				memcpy(file->rich_name, file->file_name_and_type->d_name, strlen(file->file_name_and_type->d_name) + 1);
			}
		}
	}
	else
	{
		file->rich_name = malloc(strlen(file->file_name_and_type->d_name) + 1);
		memcpy(file->rich_name, file->file_name_and_type->d_name, strlen(file->file_name_and_type->d_name) + 1);
	}
}

void get_permissions(struct file_data file, char* buf)
{
	char* ret = buf;
	*ret = (file.file_name_and_type->d_type == DT_DIR ? 'd' : '-');
	ret++;
	*ret = (file.file_data->st_mode & S_IRUSR ? 'r' : '-');
	ret++;
	*ret = (file.file_data->st_mode & S_IWUSR ? 'w' : '-');
	ret++;
	*ret = (file.file_data->st_mode & S_IXUSR ? 'x' : '-');
	ret++;

	*ret = (file.file_data->st_mode & S_IRGRP ? 'r' : '-');
	ret++;
	*ret = (file.file_data->st_mode & S_IWGRP ? 'w' : '-');
	ret++;
	*ret = (file.file_data->st_mode & S_IXGRP ? 'x' : '-');
	ret++;

	*ret = (file.file_data->st_mode & S_IROTH ? 'r' : '-');
	ret++;
	*ret = (file.file_data->st_mode & S_IWOTH ? 'w' : '-');
	ret++;
	*ret = (file.file_data->st_mode & S_IXOTH ? 'x' : '-');
	ret++;
	*ret = '\0';
}
void get_date_time(struct file_data file, char* buf)
{
	char* ret = buf;
	struct tm* time = localtime(&file.file_data->st_mtime);
	strftime(ret, 13, "%b %d %H:%M", time);
	ret[12] = '\0';
}

int find_nr_files(const char* path, char opt)
{
	DIR* directory = opendir(path);
	if(directory == NULL)
	{
		write(2, "Error opening directory for \"ls\" command.\n", 42);
		exit(2);
	}

	int files = 0;
	struct dirent* current_file_iterator;
	while((current_file_iterator = readdir(directory)) != NULL)
	{
		if ((current_file_iterator->d_name[0] != '.') || ((opt & _a) > 0))
		{
			files++;
		}
	}
	if(closedir(directory) < 0)
	{
		write(2, "Error closing directory for \"ls\" command.\n", 42);
		exit(3);
	}

	return files;
}
void get_file_data(struct file_data* file, struct dirent* current_data, const char* path)
{
	file->file_name_and_type = current_data;
	file->file_data = malloc(sizeof(struct stat));
	if(file->file_data == NULL)
	{
		write(2, "Error allocating memory for file data.\n", 39);
		exit(5);
	}

	if(stat((add_path(path, file->file_name_and_type->d_name)), file->file_data) < 0)
	{
		write(2, "Error when extrating data from file ", 36);
		write(2, file->file_name_and_type->d_name, strlen(file->file_name_and_type->d_name));
		write(2, ".\n", 2);
		exit(4);
	}
}
void get_files_from_directory(const char* path, int nr_files, char opt, struct file_data** files , int* total_block_size)
{
	DIR* directory = opendir(path);
	if(directory == NULL)
	{
		write(2, "Error opening directory for \"ls\" command.\n", 42);
		exit(2);
	}

	*files = malloc(sizeof(struct file_data) * nr_files);
	if(*files == NULL)
	{
		write(2, "Error allocating memory for \"ls\" command.\n", 42);
		exit(1);
	}


	int it = 0;
	struct dirent* current_file_iterator;
	while((current_file_iterator = readdir(directory)) != NULL)
	{
		if((current_file_iterator->d_name[0] != '.') || ((opt & _a) > 0))
		{
			get_file_data((*files)+it, current_file_iterator, path);
			
			*total_block_size += (*files)[it].file_data->st_blocks;
			it++;
		}
	}

	sort_file(*files, nr_files);

	for (int i = 0; i < nr_files; i++)
	{
		enrich_file_name((*files)+i, opt);
	}

	if(closedir(directory) < 0)
	{
		write(2, "Error closing directory for \"ls\" command.\n", 42);
		exit(3);
	}
}
void print_file_to_fd(struct file_data* files, int nr_files, int fd, char opt, int total_block_size)
{
	char* buffer;
	if(!(opt & _l))
	{
		if(opt & _s)
		{
			write(fd, "total ", 6);
			write(fd, int_to_string(total_block_size), strlen(int_to_string(total_block_size)));
			write(fd, "\n", 1);
			for(int i=0;i<nr_files;i++)
			{
				write(fd, int_to_string(files[i].file_data->st_blocks), strlen(int_to_string(files[i].file_data->st_blocks)));
				write(fd, " ", 1);
				write(fd, files[i].rich_name, strlen(files[i].rich_name));
				write(fd, "  ", 2);
			}
			write(fd, "\n", 1);
		}
		else
		{
			for(int i=0;i<nr_files;i++)
			{
				write(fd, files[i].rich_name, strlen(files[i].rich_name));
				write(fd, "  ", 2);
			}
			write(fd, "\n", 1);
		}
	}
	else
	{
		if(opt & _s)
		{
			write(fd, "total ", 6);
			write(fd, int_to_string(total_block_size), strlen(int_to_string(total_block_size)));
			write(fd, "\n", 1);
			char permissions[11];
			char date_time[13];
			for(int i=0;i<nr_files;i++)
			{
				write(fd, int_to_string(files[i].file_data->st_blocks), strlen(int_to_string(files[i].file_data->st_blocks)));
				write(fd, " ", 1);
				get_permissions(files[i], permissions);
				write(fd, permissions, 10);
				write(fd, " ", 1);
				write(fd, int_to_string(files[i].file_data->st_nlink), strlen(int_to_string(files[i].file_data->st_nlink)));
				write(fd, " ", 1);
				write(fd, getpwuid(files[i].file_data->st_uid)->pw_name, strlen(getpwuid(files[i].file_data->st_uid)->pw_name));
				write (fd, " ", 1);
				write(fd, getgrgid(files[i].file_data->st_gid)->gr_name, strlen(getgrgid(files[i].file_data->st_gid)->gr_name));
				write(fd, " ", 1);
				write(fd, int_to_string(files[i].file_data->st_size), strlen(int_to_string(files[i].file_data->st_size)));
				write(fd, " ", 1);
				get_date_time(files[i], date_time);
				write(fd, date_time, 12);
				write(fd, " ", 1);
				write(fd, files[i].rich_name, strlen(files[i].rich_name));
				write(fd, "\n", 1);
			}
			write(fd, "\n", 1);
		}
		else
		{
			char permissions[11];
			char date_time[13];
			for(int i=0;i<nr_files;i++)
			{
				get_permissions(files[i], permissions);
				write(fd, permissions, 10);
				write(fd, " ", 1);
				write(fd, int_to_string(files[i].file_data->st_nlink), strlen(int_to_string(files[i].file_data->st_nlink)));
				write(fd, " ", 1);
				write(fd, getpwuid(files[i].file_data->st_uid)->pw_name, strlen(getpwuid(files[i].file_data->st_uid)->pw_name));
				write (fd, " ", 1);
				write(fd, getgrgid(files[i].file_data->st_gid)->gr_name, strlen(getgrgid(files[i].file_data->st_gid)->gr_name));
				write(fd, " ", 1);
				write(fd, int_to_string(files[i].file_data->st_size), strlen(int_to_string(files[i].file_data->st_size)));
				write(fd, " ", 1);
				get_date_time(files[i], date_time);
				write(fd, date_time, 12);
				write(fd, " ", 1);
				write(fd, files[i].rich_name, strlen(files[i].rich_name));
				write(fd, "\n", 1);
			}
			write(fd, "\n", 1);
		}
	}
}

int main(int argc, char** argv)
{
	int option;
	opterr = 0;
	char opt = 0;
	int fd = 1;

    while((option = getopt(argc, argv, "lsaFo:")) != -1)
	{
    	switch(option)
		{
			case 'l':
				opt |= _l;
				break;
			case 's':
				opt |= _s;
				break;
			case 'a':
				opt |= _a;
				break;
			case 'F':
				opt |= _F;
				break;
			case 'o':
				opt |= _ofd;
				fd = atoi(optarg);
				break;
			case '?':
				if(optopt ==  'o')
				{
					write(2, "Option -o requires an argument.\n", 32);
					exit(5);
				}
				else
				{
					break;
				}
			default:
				write(2, "Error, unknown argument option is not accepted.\n", 48);
				exit(6);
		}
	}

	for(int i = optind; i < argc; i++)
	{
		struct file_data* files;
		int total_block_size = 0;
		int nr_files = find_nr_files(argv[i], opt);


		get_files_from_directory(argv[i], nr_files, opt, &files, &total_block_size);

		write(fd, argv[i], strlen(argv[i]));
		write(fd, ":\n", 2);

		print_file_to_fd(files, find_nr_files(argv[i], 0), 1, opt, total_block_size);
		opt |= _af;
	}


	if(opt & _af)
		return 0;
		
	int nr_files = find_nr_files(".", opt);
	int total_block_size = 0;
	struct file_data* files;

	get_files_from_directory(".", nr_files, opt, &files, &total_block_size);
	print_file_to_fd(files, nr_files, 1, opt, total_block_size);

	return 0;     
}
