#include "loginfo.h"
#include <stdio.h>
#include <string.h>

// Variable to control the threads execution for logging
// 1: Keep logging, indicates that copy threads have not finished
// 0: Stop logging, indicates that copy threads have finished
// it will be set by the main thread once it processes all the files to fill in the buffer
int keepLogging = 1;

void startLogging()
{
    printf("Starting logging...\n");
    keepLogging = 1;
}

void stopLogging()
{
    printf("Stopping logging...\n");
    keepLogging = 0;
}

LogInfo *newLogInfo()
{
    LogInfo *fileInfo = malloc(sizeof(LogInfo));
    // Example initialization
    // set fileInfo->name to an empty string without using macros or
    fileInfo->size = 0;
    fileInfo->duration = 0;
    return fileInfo;
}

void setName(LogInfo *logInfo, const char *name)
{
    logInfo->name = malloc(strlen(name) + 1);
    strcpy(logInfo->name, name);
}

void freeLogInfo(LogInfo *logInfo)
{
    free(logInfo->name);
    free(logInfo);
}

char *toStringLogInfo(LogInfo *logInfo)
{
    char *str = malloc(256 * sizeof(char));
    snprintf(str, 256, "LogInfo: %s. Size %ld. Duration %.2f ms.", logInfo->name, logInfo->size, logInfo->duration);
    return str;
}

LogInfoBuffer *newLogInfoBuffer()
{
    LogInfoBuffer *logInfoBuffer = malloc(sizeof(LogInfoBuffer));
    pthread_mutex_init(&logInfoBuffer->mutex, NULL);
    pthread_cond_init(&logInfoBuffer->not_full, NULL);
    pthread_cond_init(&logInfoBuffer->not_empty, NULL);
    logInfoBuffer->buffer = malloc(BUFFER_SIZE * sizeof(LogInfo));
    logInfoBuffer->readIndex = 0;
    logInfoBuffer->writeIndex = 0;
    return logInfoBuffer;
}

void freeLogInfoBuffer(LogInfoBuffer *logInfoBuffer)
{
    free(logInfoBuffer->buffer);
    pthread_mutex_destroy(&logInfoBuffer->mutex);
    pthread_cond_destroy(&logInfoBuffer->not_full);
    pthread_cond_destroy(&logInfoBuffer->not_empty);
    free(logInfoBuffer);
}
/*
writeFileInfo writes a FileInfo struct to the buffer
*/
void writeLogInfo(LogInfoBuffer *logInfoBuffer, LogInfo *logInfo)
{
    pthread_mutex_lock(&logInfoBuffer->mutex); // Lock the mutex

    // Wait until there is space in the buffer
    while (isFullLogInfo(logInfoBuffer))
    {
        printf("LogInfo Buffer is full, waiting for a read\n");
        pthread_cond_wait(&logInfoBuffer->not_full, &logInfoBuffer->mutex);
    }
    printf("writing LogInfo '%s' into buffer[%d]\n", logInfo->name, logInfoBuffer->writeIndex);
    logInfoBuffer->buffer[logInfoBuffer->writeIndex] = *logInfo;               // Write the struct to the buffer,
    logInfoBuffer->writeIndex = (logInfoBuffer->writeIndex + 1) % BUFFER_SIZE; // Increase the write index, and uses the modulo operator to keep it in the range [0, BUFFER_SIZE)

    pthread_cond_signal(&logInfoBuffer->not_empty); // Signal that a new item has been written
    pthread_mutex_unlock(&logInfoBuffer->mutex);    // Unlock the mutex
}

LogInfo *readLogInfo(LogInfoBuffer *logInfoBuffer)
{
    pthread_mutex_lock(&logInfoBuffer->mutex); // Lock the mutex
    // wait while the buffer is empty and copy is still running (keepLogging == 1)
    while (!hasLogInfo(logInfoBuffer) && keepLogging)
    {
        printf("LogInfo Buffer is empty, waiting for a write\n");
        pthread_cond_wait(&logInfoBuffer->not_empty, &logInfoBuffer->mutex);
    }
    printf("reading LogInfo '%s' from buffer[%d]\n", logInfoBuffer->buffer[logInfoBuffer->readIndex].name, logInfoBuffer->readIndex);
    LogInfo *logInfo = &logInfoBuffer->buffer[logInfoBuffer->readIndex];
    logInfoBuffer->readIndex = (logInfoBuffer->readIndex + 1) % BUFFER_SIZE;
    pthread_cond_signal(&logInfoBuffer->not_full);
    pthread_mutex_unlock(&logInfoBuffer->mutex);
    return logInfo;
}

int hasLogInfo(LogInfoBuffer *logInfoBuffer)
{
    return logInfoBuffer->writeIndex != logInfoBuffer->readIndex;
}

int isEmptyLogInfo(LogInfoBuffer *logInfoBuffer)
{
    return logInfoBuffer->writeIndex == logInfoBuffer->readIndex;
}

int isFullLogInfo(LogInfoBuffer *logInfoBuffer)
{
    return ((logInfoBuffer->writeIndex + 1) % BUFFER_SIZE) == logInfoBuffer->readIndex;
}