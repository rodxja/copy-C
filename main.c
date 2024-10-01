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

// it receives in argv[1] the source directory and in argv[2] the destination directory
// in argv[3] the number of threads to create for copy
// in argv[4] the name of the log file

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        fprintf(stderr, "Usage: %s <source_dir> <dest_dir>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *sourceDir = argv[1]; // Source directory where are contained the files to copy
    char *destDir = argv[2];   // Destination directory where the files will be copied

    char *num_threads_str = argv[3]; // Number of threads to create for copy
    int NUM_THREADS = atoi(num_threads_str);

    char *logFileName = argv[4]; // Name of the log file

    // print all argv
    printf("Source directory: %s, Destination directory %s, Threads %d, Log %s\n", sourceDir, destDir, NUM_THREADS, logFileName);

    // const int NUM_THREADS = 4; // Number of threads to create for copy
    // + 2 thread for readDirectory and writeLog
    int numThreads = NUM_THREADS + 2; // Number of threads to create

    // BUFFERS
    FILE_INFO_BUFFER = newFileInfoBuffer();
    LOG_INFO_BUFFER = newLogInfoBuffer();

    // Create the readDirectoryInfo struct
    ReadDirectoryInfo *readDirectoryInfo = newReadDirectoryInfo();
    readDirectoryInfo->origin = sourceDir;
    readDirectoryInfo->destination = destDir;
    readDirectoryInfo->threadNum = 0;

    // Create threads
    pthread_t threads[numThreads];
    int threadIds[numThreads]; // Array to store the thread ids, gives a number to each thread to identify them

    startCopying(FILE_INFO_BUFFER);
    // create thread[0] for readDirectory
    threadIds[0] = 0;
    pthread_create(&threads[0], NULL, readDirectory, readDirectoryInfo);

    startLogging(LOG_INFO_BUFFER);
    // Create the threads[1:n-2] for copy
    for (int i = 1; i < numThreads - 1; i++)
    {
        threadIds[i] = i;
        pthread_create(&threads[i], NULL, copy, &threadIds[i]); // Missing the copyFiles function
    }

    // Create the thread[n-1] for writeLog
    threadIds[numThreads - 1] = numThreads - 1;
    WriteLogInfo *writeLogInfo = newWriteLogInfo();
    writeLogInfo->logFile = logFileName;
    writeLogInfo->threadNum = numThreads - 1;
    pthread_create(&threads[numThreads - 1], NULL, writeLog, writeLogInfo);

    // waiting for the readDirectory thread to finish
    void *resultReadDirectory;
    pthread_join(threads[0], &resultReadDirectory);
    int threadNumReadDirectory = (int)(size_t)resultReadDirectory;

    stopCopying(FILE_INFO_BUFFER);

    // Main thread waits for his son threads to finish, before he continues
    // NUM_THREADS for copy and 1 for writeLog
    for (int i = 1; i < numThreads - 1; i++) // try this one
    {
        void *result;
        pthread_join(threads[i], &result);
        int threadNum = (int)(size_t)result;
    }

    // now that the threads have finished, we can stop the logging
    stopLogging(LOG_INFO_BUFFER);

    // wait for the writeLog thread to finish
    void *result;
    pthread_join(threads[numThreads - 1], &result);
    int threadNum = (int)(size_t)result;

    printf("Copy finished.\n");
    return 0;
}

// gcc -o main main.c fileinfo.c loginfo.c functions.c -pthread
// ./main ./test ./destination