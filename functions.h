#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fileinfo.h"
#include "loginfo.h"

// BUFFERS
extern FileInfoBuffer *FILE_INFO_BUFFER;
extern LogInfoBuffer *LOG_INFO_BUFFER;

typedef struct
{
    char *origin;      // Field to store the origin path
    char *destination; // Field to store the destination path
    int threadNum;     // Field to store the thread number
} ReadDirectoryInfo;

ReadDirectoryInfo *newReadDirectoryInfo();

// Function prototypes
void *readDirectory(void *arg);
void *copy(void *arg);
void *writeLog(void *arg);

#endif // FUNCTIONS_H
