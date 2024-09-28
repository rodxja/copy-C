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

// Variable to control the threads execution
// 1: Keep copying
// 0: Stop copying
// it will be set by the main thread once it processes all the files to fill in the buffer
int keepCopying = 1;

// BUFFERS
FileInfoBuffer *FILE_INFO_BUFFER;
LogInfoBuffer *LOG_INFO_BUFFER;

/*
readDirectory reads the files in a directory and stores the information in the FILE_INFO_BUFFER
*/
void readDirectory(const char *sourceDir, const char *destDir)
{
    DIR *dir;
    struct dirent *entry; // Entries in the directory, files or subdirectories
    struct stat statbuf;  // Struct to store file information, like size

    dir = opendir(sourceDir);

    if (dir == NULL)
    {
        perror("Error opening directory");
        exit(EXIT_FAILURE);
    }

    // filesBuffer = malloc(100 * sizeof(FileInfo));

    while ((entry = readdir(dir)) != NULL)
    { // Iterate over the entries in the directory
        char sourcePath[MAX_NAME_LENGTH];
        char destPath[MAX_NAME_LENGTH];

        FileInfo *fileInfo = malloc(sizeof(FileInfo));

        // Create the full path for the source and destination files
        snprintf(sourcePath, sizeof(sourcePath), "%s/%s", sourceDir, entry->d_name);
        snprintf(destPath, sizeof(destPath), "%s/%s", destDir, entry->d_name);

        // Fill the buffer with the file information
        if (stat(sourcePath, &statbuf) == 0 && S_ISREG(statbuf.st_mode))
        {
            strcpy(fileInfo->origin, sourcePath);
            strcpy(fileInfo->destination, destPath);

            fileInfo->size = statbuf.st_size;

            writeFileInfo(FILE_INFO_BUFFER, fileInfo);
        }
    }

    closedir(dir);
}

void *copyFiles(void *arg)
{
    int threadNum = *((int *)arg);

    while (keepCopying)
    {
        if (FILE_INFO_BUFFER == NULL)
        {
            printf("Error: fileNameBuffer is null. Thread '#%ld' is going to stop.\n", threadNum);
            return;
        }

        LogInfo *logInfo = newLogInfo();

        FileInfo *fileInfo = readFileInfo(FILE_INFO_BUFFER);
        // !!! i am not using size

        snprintf(logInfo->name, MAX_NAME_LENGTH, "%s", fileInfo->destination);

        // Copy the file
        int sourceFD = open(fileInfo->origin, O_RDONLY);
        if (sourceFD == -1)
        {
            printf("Error: opening source file '%s' on thread '#%ld'.\n", fileInfo->origin, threadNum);
            continue;
        }

        // ??? I THINK THIS DESTINATION SHOULD CHANGE
        int destFD = open(fileInfo->destination, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (destFD == -1)
        {
            perror("Error opening destination file"); // TODO : extend print
            close(sourceFD);
            continue;
        }

        char buffer[1024];
        ssize_t bytesRead;
        clock_t start = clock();
        while ((bytesRead = read(sourceFD, buffer, sizeof(buffer))) > 0)
        {
            fileInfo->size += bytesRead;
            if (write(destFD, buffer, bytesRead) != bytesRead)
            {
                perror("Error writing to destination file"); // TODO : extend print
                close(sourceFD);
                close(destFD);
                break;
            }
        }
        clock_t end = clock();

        logInfo->duration = (double)(end - start) * 1000 / CLOCKS_PER_SEC;

        close(sourceFD);
        close(destFD);

        // Lock the mutex to increment the filesCopied counter
        writeLogInfo(LOG_INFO_BUFFER, logInfo);
    }

    return NULL;
}

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