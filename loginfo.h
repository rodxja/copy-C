#ifndef LOGINFO_H
#define LOGINFO_H

#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include "macros.h"

extern int keepLogging;

void startLogging();
void stopLogging();

/*
LogInfo stores the information of a file that was copied
It has the name, the size in bytes and the duration in miliseconds
*/
typedef struct
{
    char *name;      // Field to store the name
    size_t size;     // In bytes
    double duration; // In miliseconds
} LogInfo;

LogInfo *newLogInfo();
void setName(LogInfo *logInfo, const char *name);
void freeLogInfo(LogInfo *logInfo);
char *toStringLogInfo(LogInfo *logInfo);

/*
FileInfoBuffer is the buffer that will store the FileInfo structs
It contains a mutex to control the access to the buffer, the buffer itself
and two indexes, one for reading and one for writing
that will be increased by 1 each time a struct is read or written
*/
typedef struct
{
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;

    LogInfo *buffer; // Dynamic buffer for structs
    int readIndex;
    int writeIndex;

} LogInfoBuffer;

// Function prototypes
LogInfoBuffer *newLogInfoBuffer();
void freeLogInfoBuffer(LogInfoBuffer *logInfoBuffer);
void writeLogInfo(LogInfoBuffer *logInfoBuffer, LogInfo *logInfo);
LogInfo *readLogInfo(LogInfoBuffer *logInfoBuffer);
int hasLogInfo(LogInfoBuffer *logINfoBuffer);
int isEmptyLogInfo(LogInfoBuffer *logInfoBuffer);
int isFullLogInfo(LogInfoBuffer *logInfoBuffer);

#endif // LOGINFO_H
