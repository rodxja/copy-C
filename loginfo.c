#include "loginfo.h"
#include <stdio.h>
#include <string.h>

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

char *toCSVHeaderLogInfo()
{
    char *str = malloc(256 * sizeof(char));
    snprintf(str, 256, "Name,Size,Duration\n");
    return str;
}

char *toCSVLogInfo(LogInfo *logInfo)
{
    char *str = malloc(256 * sizeof(char));
    snprintf(str, 256, "%s,%ld,%.2f\n", logInfo->name, logInfo->size, logInfo->duration);
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
    logInfoBuffer->totalBytes = 0;
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
        pthread_cond_wait(&logInfoBuffer->not_full, &logInfoBuffer->mutex);
    }
    logInfoBuffer->buffer[logInfoBuffer->writeIndex] = *logInfo;               // Write the struct to the buffer,
    logInfoBuffer->writeIndex = (logInfoBuffer->writeIndex + 1) % BUFFER_SIZE; // Increase the write index, and uses the modulo operator to keep it in the range [0, BUFFER_SIZE)

    pthread_cond_broadcast(&logInfoBuffer->not_empty); // Signal that a new item has been written
    pthread_mutex_unlock(&logInfoBuffer->mutex);       // Unlock the mutex
}

LogInfo *readLogInfo(LogInfoBuffer *logInfoBuffer)
{
    pthread_mutex_lock(&logInfoBuffer->mutex); // Lock the mutex
    // wait while the buffer is empty and copy is still running (keepLogging == 1)
    while (isEmptyLogInfo(logInfoBuffer) && logInfoBuffer->keepLogging)
    {
        pthread_cond_wait(&logInfoBuffer->not_empty, &logInfoBuffer->mutex);
    }
    // check that the buffer is not empty
    if (isEmptyLogInfo(logInfoBuffer))
    {
        pthread_mutex_unlock(&logInfoBuffer->mutex);
        return NULL;
    }
    LogInfo *logInfo = &logInfoBuffer->buffer[logInfoBuffer->readIndex];
    logInfoBuffer->readIndex = (logInfoBuffer->readIndex + 1) % BUFFER_SIZE;
    logInfoBuffer->totalBytes += logInfo->size;
    pthread_cond_broadcast(&logInfoBuffer->not_full);
    pthread_mutex_unlock(&logInfoBuffer->mutex);
    return logInfo;
}

int isEmptyLogInfo(LogInfoBuffer *logInfoBuffer)
{
    return logInfoBuffer->writeIndex == logInfoBuffer->readIndex;
}

// this function affects that BUFFER_SIZE should not be 1
int isFullLogInfo(LogInfoBuffer *logInfoBuffer)
{
    return ((logInfoBuffer->writeIndex + 1) % BUFFER_SIZE) == logInfoBuffer->readIndex;
}

void startLogging(LogInfoBuffer *logInfoBuffer)
{
    logInfoBuffer->keepLogging = 1;
}

void stopLogging(LogInfoBuffer *logInfoBuffer)
{
    pthread_mutex_lock(&logInfoBuffer->mutex);
    pthread_cond_broadcast(&logInfoBuffer->not_empty);
    logInfoBuffer->keepLogging = 0;
    pthread_mutex_unlock(&logInfoBuffer->mutex);
}
