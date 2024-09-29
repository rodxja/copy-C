#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include "fileinfo.h"
#include "loginfo.h"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <source_dir> <dest_dir>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *sourceDir = argv[1]; // Source directory where are contained the files to copy
    const char *destDir = argv[2];   // Destination directory where the files will be copied

    readDirectory(sourceDir, destDir);

    // Create threads
    pthread_t threads[NUM_THREADS];
    int threadIds[NUM_THREADS]; // Array to store the thread ids, gives a number to each thread to identify them

    for (int i = 0; i < NUM_THREADS; i++)
    {
        threadIds[i] = i;
        pthread_create(&threads[i], NULL, copyFiles, &threadIds[i]); // Missing the copyFiles function
    }

    // Main thread waits for his son threads to finish, before he continues
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return 0;
}

// gcc -o mainCopy mainCopy.c fileinfo.c loginfo.c -pthread