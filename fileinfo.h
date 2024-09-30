#ifndef FILEINFO_H
#define FILEINFO_H

#include <pthread.h>
#include <stdlib.h>
#include "macros.h"

/*
FileInfo stores the information of a file to be copied
It has the origin path, the destination path and the size in bytes
*/
typedef struct
{
    char *origin;      // Field to store the origin path
    char *destination; // Field to store the destination path
    size_t size;       // Field to store the size in bytes
} FileInfo;

FileInfo *newFileInfo();
void setOrigin(FileInfo *fileInfo, const char *origin);
void setDestination(FileInfo *fileInfo, const char *destination);
void freeFileInfo(FileInfo *fileInfo);
char *toStringFileInfo(FileInfo *fileInfo);

// Struct definition for FileInfoBuffer
typedef struct
{
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
    FileInfo *buffer; // Dynamic buffer for structs
    int readIndex;
    int writeIndex;
    // Variable to control the threads execution for copying files
    // 1: Keep copying, indicates that readDirectory has not finished
    // 0: Stop copying, indicates that readDirectory has finished
    // it will be set by the main thread once it processes all the files to fill in the buffer
    int keepCopying;
} FileInfoBuffer;

// Function prototypes
FileInfoBuffer *newFileInfoBuffer();
void freeFileInfoBuffer(FileInfoBuffer *logInfoBuffer);
void writeFileInfo(FileInfoBuffer *logInfoBuffer, FileInfo *logInfo);
FileInfo *readFileInfo(FileInfoBuffer *logInfoBuffer);
int isEmptyFileInfo(FileInfoBuffer *fileInfoBuffer);
int isFullFileInfo(FileInfoBuffer *fileInfoBuffer);

void startCopying(FileInfoBuffer *fileInfoBuffer);
void stopCopying(FileInfoBuffer *fileInfoBuffer);

#endif // FILEINFO_H
