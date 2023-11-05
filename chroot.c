#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s NEW_ROOT\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *new_root = argv[1];

    if (chroot(new_root) == -1)
    {
        perror("chroot");
        exit(EXIT_FAILURE);
    }

    if (chdir("/") == -1)
    {
        perror("chdir");
        exit(EXIT_FAILURE);
    }

    system("./terminal");

    return 0;
}