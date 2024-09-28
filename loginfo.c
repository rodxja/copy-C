#include "loginfo.h"
#include <stdio.h>
#include <string.h>

LogInfo *newLogInfo(/* char *name, size_t size, time_t timestamp */)
{
    LogInfo *fileInfo = malloc(sizeof(LogInfo));
    // Example initialization
    snprintf(fileInfo->name, sizeof(fileInfo->name), "default_log");
    fileInfo->size = 0;
    fileInfo->duration = 0;
    return fileInfo;
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
void writeLogInfo(LogInfoBuffer *logInfoBuffer, LogInfo *logInfo) // !! it receives a pointer
{
    pthread_mutex_lock(&logInfoBuffer->mutex); // Lock the mutex

    // Wait until there is space in the buffer
    while (((logInfoBuffer->writeIndex + 1) % BUFFER_SIZE) == logInfoBuffer->readIndex)
    {
        pthread_cond_wait(&logInfoBuffer->not_full, &logInfoBuffer->mutex);
    }
    // !!! logInfo is being dereferenced
    logInfoBuffer->buffer[logInfoBuffer->writeIndex] = *logInfo;               // Write the struct to the buffer,
    logInfoBuffer->writeIndex = (logInfoBuffer->writeIndex + 1) % BUFFER_SIZE; // Increase the write index, and uses the modulo operator to keep it in the range [0, BUFFER_SIZE)

    pthread_cond_signal(&logInfoBuffer->not_empty); // Signal that a new item has been written
    pthread_mutex_unlock(&logInfoBuffer->mutex);    // Unlock the mutex
}

LogInfo *readLogInfo(LogInfoBuffer *logInfoBuffer)
{
    pthread_mutex_lock(&logInfoBuffer->mutex); // Lock the mutex
    while (logInfoBuffer->writeIndex == logInfoBuffer->readIndex)
    {
        pthread_cond_wait(&logInfoBuffer->not_empty, &logInfoBuffer->mutex);
    }
    LogInfo *logInfo = &logInfoBuffer->buffer[logInfoBuffer->readIndex];
    logInfoBuffer->readIndex = (logInfoBuffer->readIndex + 1) % BUFFER_SIZE;
    pthread_cond_signal(&logInfoBuffer->not_full);
    pthread_mutex_unlock(&logInfoBuffer->mutex);
    return logInfo;
}
