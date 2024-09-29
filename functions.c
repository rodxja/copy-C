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
// #include <libgen.h> // For dirname()
#include "functions.h"
#include "fileinfo.h"
#include "loginfo.h"

// Variable to control the threads execution for copying files
// 1: Keep copying
// 0: Stop copying
// it will be set by the main thread once it processes all the files to fill in the buffer
int keepCopying = 1;

// Variable to control the threads execution for logging
// 1: Keep logging
// 0: Stop logging
// it will be set by the main thread once it processes all the files to fill in the buffer
int keepLogging = 1;

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

    // create_directories_except_last(destDir);

    dir = opendir(sourceDir);
    if (dir == NULL)
    {
        perror("Error opening directory");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL)
    {
        char *sourcePath;
        char *destPath;

        // Skip the current directory and the parent directory
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        // Memory allocation for the paths
        size_t srcLen = strlen(sourceDir) + strlen(entry->d_name) + 2;
        sourcePath = (char *)malloc(srcLen * sizeof(char)); // +2 for the '/' and the null terminator
        size_t destLen = strlen(destDir) + strlen(entry->d_name) + 2;
        destPath = (char *)malloc(destLen * sizeof(char));

        // set sourcePath and destPath
        snprintf(sourcePath, srcLen, "%s/%s", sourceDir, entry->d_name);
        snprintf(destPath, destLen, "%s/%s", destDir, entry->d_name);
        // Fill the buffer with the file information

        if (stat(sourcePath, &statbuf) == 0)
        {
            // Validate if the entry is a file
            if (S_ISREG(statbuf.st_mode))
            {
                FileInfo *fileInfo = newFileInfo();
                setOrigin(fileInfo, sourcePath);
                setDestination(fileInfo, destPath);
                fileInfo->size = statbuf.st_size;

                printf("'readDirectory' is ");
                writeFileInfo(FILE_INFO_BUFFER, fileInfo);

                free(sourcePath);
                free(destPath);
            }
            // Validate if the entry is a directory
            else if (S_ISDIR(statbuf.st_mode))
            {
                // Create the directory in the destination path
                // Move the directory creation to the copy function
                // Destination must not exists in order to copy the files
                // printf("Creating directory '%s'.\n", destPath);
                mkdir(destPath, 0755);

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

    // copy while there are files to copy
    // as there may several threads running this 'copy' function
    // there will be a moment where hasFileInfo will return 0 and stops the threads
    // this is because there is ONE single thread filling the buffer, against n threads reading from it
    // so we need another condition to keep the threads running
    // keepCopying is set to 0 by the main thread once it finishes reading the directory
    // then the FILE_INFO_BUFFER will have files info until the last thread reads the last file
    // and then the threads will stop one by one
    while (hasFileInfo(FILE_INFO_BUFFER) || keepCopying)
    {
        if (FILE_INFO_BUFFER == NULL)
        {
            printf("Error: fileNameBuffer is null. Thread '%d' is going to stop.\n", threadNum);
            return (void *)(size_t)threadNum;
        }

        LogInfo *logInfo = newLogInfo();

        printf("'Copy' Thread '%d' is ", threadNum);
        FileInfo *fileInfo = readFileInfo(FILE_INFO_BUFFER);

        // Set the name of the file to the logInfo
        setName(logInfo, fileInfo->destination);

        // Copy the file
        int sourceFD = open(fileInfo->origin, O_RDONLY);
        if (sourceFD == -1)
        {
            printf("Error: opening source file '%s' on thread '%d'.\n", fileInfo->origin, threadNum);
            continue;
        }

        // printf("Copying file '%s' to '%s' on thread '%d'.\n", fileInfo->origin, fileInfo->destination, threadNum);
        // create_directories_except_last(fileInfo->destination);

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

            logInfo->size += bytesRead; // add sizes

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
        }
        clock_t end = clock();

        logInfo->duration = (double)(end - start) * 1000 / CLOCKS_PER_SEC;

        close(sourceFD);
        close(destFD);

        // Lock the mutex to increment the filesCopied counter
        printf("'Copy' Thread '%d' is ", threadNum);
        writeLogInfo(LOG_INFO_BUFFER, logInfo);
        // freeFileInfo(fileInfo);
    }

    return (void *)(size_t)threadNum;
}

// this function will be called by one thread
void *writeLog(void *arg)
{
    int threadNum = *((int *)arg);

    // keep loggin while there is log info to read or keepLogging is set to 1
    // keepLogging is set to 0 by the main thread once it finishes copying the files
    // then the LOG_INFO_BUFFER will have log info until the last thread reads the last log info
    // and then the threads will stop one by one

    // this will not affect that much due that there will be n threads filling the buffer against one thread reading from it
    while (hasLogInfo(LOG_INFO_BUFFER) || keepLogging)
    {
        printf("'WriteLog' Thread '%d' is ", threadNum);
        LogInfo *logInfo = readLogInfo(LOG_INFO_BUFFER);

        // create a csv. file with the log info

        FILE *csvFile = fopen("log.csv", "a");
        if (csvFile == NULL)
        {
            printf("Error opening log.csv file on thread '%d'.\n", threadNum);
            continue;
        }

        fprintf(csvFile, "%s,%ld,%f\n", logInfo->name, logInfo->size, logInfo->duration);

        fclose(csvFile);

        // freeLogInfo(logInfo); // do not free for now
    }

    return (void *)(size_t)threadNum;
}