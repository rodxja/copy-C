#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argn, char *argv[])
{

    int src_fd, dst_fd, n, err;
    unsigned char buffer[4096];
    char *src_path, *dst_path;

    // Assume that the program takes two arguments the source path followed
    // by the destination path.

    if (argn != 3)
    {
        printf("Wrong argument count.\n");
        exit(1);
    }

    src_path = argv[1];
    dst_path = argv[2];

    src_fd = open(src_path, O_RDONLY);
    dst_fd = open(dst_path, O_CREAT | O_WRONLY | O_EXCL, 0666);

    while (1)
    {
        err = read(src_fd, buffer, 4096);
        if (err == -1)
        {
            printf("Error reading file.\n");
            exit(1);
        }
        n = err;

        if (n == 0)
            break;

        err = write(dst_fd, buffer, n);
        if (err == -1)
        {
            printf("Error writing to file.\n");
            exit(1);
        }
    }

    close(src_fd);
    close(dst_fd);
}

//  gcc -o mainCopy mainCopy.c fileinfo.c loginfo.c -pthread