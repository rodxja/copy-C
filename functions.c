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
#include <errno.h>
#include "functions.h"
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

    while ((entry = readdir(dir)) != NULL)
    {
        char sourcePath[MAX_NAME_LENGTH];
        char destPath[MAX_NAME_LENGTH];

        // Skip the current directory and the parent directory
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        // Create the full path for the source and destination files
        snprintf(sourcePath, sizeof(sourcePath), "%s/%s", sourceDir, entry->d_name);
        snprintf(destPath, sizeof(destPath), "%s/%s", destDir, entry->d_name);

        // Fill the buffer with the file information
        if (stat(sourcePath, &statbuf) == 0)
        {
            // Validate if the entry is a file
            if (S_ISREG(statbuf.st_mode))
            {
                FileInfo *fileInfo = malloc(sizeof(FileInfo)); // Memory allocation for the struct
                strcpy(fileInfo->origin, sourcePath);
                strcpy(fileInfo->destination, destPath);
                fileInfo->size = statbuf.st_size;

                writeFileInfo(FILE_INFO_BUFFER, fileInfo);
            }
            // Validate if the entry is a directory
            else if (S_ISDIR(statbuf.st_mode))
            {
                // Create the directory in the destination path
                // !!! i think that readDirectory should not create directories
                // mkdir(destPath, 0755);

                // Recursive call to read the directory
                readDirectory(sourcePath, destPath);
            }
        }
    }
    closedir(dir);
}

// https://stackoverflow.com/questions/7267295/how-can-i-copy-a-file-from-one-directory-to-another-in-c-c
void *copy(void *arg)
{
    int threadNum = *((int *)arg);

    // !!! evaluate other ways to stop the threads
    // copy while there are files to copy
    while (hasFiles(FILE_INFO_BUFFER)) // change this keepCopying to a global variable in FILE_INFO_BUFFER
    {
        if (FILE_INFO_BUFFER == NULL)
        {
            printf("Error: fileNameBuffer is null. Thread '%d' is going to stop.\n", threadNum);
            return NULL;
        }

        LogInfo *logInfo = newFileInfo();

        FileInfo *fileInfo = readFileInfo(FILE_INFO_BUFFER);
        // !!! i am not using size

        snprintf(logInfo->name, MAX_NAME_LENGTH, "%s", fileInfo->destination);

        // Copy the file
        int sourceFD = open(fileInfo->origin, O_RDONLY);
        if (sourceFD == -1)
        {
            printf("Error: opening source file '%s' on thread '%d'.\n", fileInfo->origin, threadNum);
            continue;
        }

        printf("Thread '%d' is copying file '%s' to '%s'.\n", threadNum, fileInfo->origin, fileInfo->destination);
        // creates destination recursively
        fileInfo->destination[strlen(fileInfo->destination)] = '\0'; // Make sure the string is null-terminated
        char *tmpPath = strdup(fileInfo->destination);               // Duplicate the path so we can modify it

        printf("tmpPath: %s\n", tmpPath);
        char *currentDir = dirname(tmpPath); // Get the directory part of the path

        printf("currentDir: %s\n", currentDir);

        // Recursively create directories if they don't exist
        if (mkdir(currentDir, 0777) == -1)
        {
            if (errno != EEXIST)
            {
                // If directory creation failed and it's not because it already exists
                fprintf(stderr, "Error creating directory %s: %s\n", currentDir, strerror(errno));
                free(tmpPath);
                return NULL;
            }
        }

        free(tmpPath); // Clean up the temporary string

        // ??? I THINK THIS DESTINATION SHOULD CHANGE
        int destFD = open(fileInfo->destination, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (destFD == -1)
        {
            perror("Error: opening destination file"); // TODO : extend print
            close(sourceFD);
            continue;
        }

        char buffer[4096];
        clock_t start = clock();

        // copy while there is something to read or an error occurs
        while (1)
        {
            ssize_t bytesRead = read(sourceFD, buffer, sizeof(buffer));
            if (bytesRead == 0)
            { // End of file
                printf("End of file '%s' on thread '%d'.\n", fileInfo->origin, threadNum);
                break;
            }
            if (bytesRead == -1)
            {
                char errorMsg[256];
                sprintf(errorMsg, "Error reading from source file '%s' on thread '%d'", fileInfo->origin, threadNum);
                perror(errorMsg);
                close(sourceFD);
                close(destFD);
                break;
            }

            fileInfo->size += bytesRead; // add sizes

            ssize_t bytesWritten = write(destFD, buffer, bytesRead);
            if (bytesWritten == -1)
            {
                char errorMsg[256];
                sprintf(errorMsg, "Error writing to destination file '%s' on thread '%d'.\n", fileInfo->destination, threadNum);
                perror(errorMsg);
                close(sourceFD);
                close(destFD);
                break;
            }

            if (bytesRead != bytesWritten)
            {
                char errorMsg[256];
                sprintf(errorMsg, "Error: read %ld bytes but wrote %ld bytes on thread '%d. To destination '%s''.\n", bytesRead, bytesWritten, threadNum, fileInfo->destination);
                perror(errorMsg);
                close(sourceFD);
                close(destFD);
                break;
            }

            printf("Thread '%d' copied %ld bytes from '%s' to '%s'.\n", threadNum, bytesRead, fileInfo->origin, fileInfo->destination);
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

void *writeLog(void *arg)
{
}