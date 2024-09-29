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
#include "functions.h"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <source_dir> <dest_dir>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // TODO : once readDirectory is handle as a single thread, add another thread to numThreads
    // const int NUM_THREADS = 4; // Number of threads to create for copy
    // + 1 thread for writeLog
    int numThreads = NUM_THREADS + 1; // Number of threads to create

    const char *sourceDir = argv[1]; // Source directory where are contained the files to copy
    const char *destDir = argv[2];   // Destination directory where the files will be copied

    // BUFFERS
    FILE_INFO_BUFFER = newFileInfoBuffer();
    LOG_INFO_BUFFER = newLogInfoBuffer();

    printf("Copying files from '%s' to '%s' using %d threads.\n", sourceDir, destDir, numThreads - 1);
    // TODO :  handle as single thread
    readDirectory(sourceDir, destDir);

    // now that readDirectory has finished, we can stop the threads
    keepCopying = 0;

    // Create threads
    pthread_t threads[numThreads];
    int threadIds[numThreads]; // Array to store the thread ids, gives a number to each thread to identify them

    // TODO : start i from 1 when readDirectory is handle as a single thread
    for (int i = 0; i < numThreads - 1; i++)
    {
        threadIds[i] = i;
        pthread_create(&threads[i], NULL, copy, &threadIds[i]); // Missing the copyFiles function
    }

    // Create the thread for writeLog
    threadIds[numThreads - 1] = numThreads - 1;
    pthread_create(&threads[numThreads - 1], NULL, writeLog, &threadIds[numThreads - 1]);

    // Main thread waits for his son threads to finish, before he continues
    // NUM_THREADS for copy and 1 for writeLog
    // TODO : pending to add thread for readDirectory
    for (int i = 0; i < NUM_THREADS; i++)
    {
        void *result;
        pthread_join(threads[i], &result);
        int threadNum = (int)(size_t)result;
        printf("Copy thread %d has stopped.\n", threadNum);
    }

    // now that the threads have finished, we can stop the logging
    keepLogging = 0;

    // wait for the writeLog thread to finish
    void *result;
    pthread_join(threads[numThreads - 1], &result);
    int threadNum = (int)(size_t)result;
    printf("Log thread %d has stopped.\n", threadNum);

    return 0;
}

// gcc -o main main.c fileinfo.c loginfo.c functions.c -pthread
// ./main ./test ./copy/more