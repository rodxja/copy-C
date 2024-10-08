#include "fileinfo.h"
#include <stdio.h>
#include <string.h>

FileInfo *newFileInfo()
{
    FileInfo *fileInfo = malloc(sizeof(FileInfo));
    fileInfo->size = 0;
    return fileInfo;
}

void setOrigin(FileInfo *fileInfo, const char *origin)
{
    fileInfo->origin = malloc(strlen(origin) + 1);
    strcpy(fileInfo->origin, origin);
}

void setDestination(FileInfo *fileInfo, const char *destination)
{
    fileInfo->destination = malloc(strlen(destination) + 1);
    strcpy(fileInfo->destination, destination);
}

void freeFileInfo(FileInfo *fileInfo)
{
    free(fileInfo->origin);
    free(fileInfo->destination);
    free(fileInfo);
}

char *toStringFileInfo(FileInfo *fileInfo)
{
    char *str = malloc(256 * sizeof(char));
    snprintf(str, 256, "FileInfo: %s -> %s. Size %ld.", fileInfo->origin, fileInfo->destination, fileInfo->size);
    return str;
}

FileInfoBuffer *newFileInfoBuffer()
{
    FileInfoBuffer *fileInfoBuffer = malloc(sizeof(FileInfoBuffer));
    pthread_mutex_init(&fileInfoBuffer->mutex, NULL);
    pthread_cond_init(&fileInfoBuffer->not_full, NULL);
    pthread_cond_init(&fileInfoBuffer->not_empty, NULL);
    fileInfoBuffer->buffer = malloc(BUFFER_SIZE * sizeof(FileInfo));
    fileInfoBuffer->readIndex = 0;
    fileInfoBuffer->writeIndex = 0;
    fileInfoBuffer->keepCopying = 1;
    return fileInfoBuffer;
}

void freeFileInfoBuffer(FileInfoBuffer *fileInfoBuffer)
{
    free(fileInfoBuffer->buffer);
    pthread_mutex_destroy(&fileInfoBuffer->mutex);
    pthread_cond_destroy(&fileInfoBuffer->not_full);
    pthread_cond_destroy(&fileInfoBuffer->not_empty);
    free(fileInfoBuffer);
}

void writeFileInfo(FileInfoBuffer *fileInfoBuffer, FileInfo *fileInfo)
{
    pthread_mutex_lock(&fileInfoBuffer->mutex);
    // wait while the buffer is full
    while (isFullFileInfo(fileInfoBuffer))
    {
        pthread_cond_wait(&fileInfoBuffer->not_full, &fileInfoBuffer->mutex);
    }
    fileInfoBuffer->buffer[fileInfoBuffer->writeIndex] = *fileInfo;
    fileInfoBuffer->writeIndex = (fileInfoBuffer->writeIndex + 1) % BUFFER_SIZE;
    pthread_cond_broadcast(&fileInfoBuffer->not_empty);
    pthread_mutex_unlock(&fileInfoBuffer->mutex);
}

FileInfo *readFileInfo(FileInfoBuffer *fileInfoBuffer)
{
    pthread_mutex_lock(&fileInfoBuffer->mutex);
    // wait while the buffer is empty and readDirectory is still running (keepCopying == 1)
    while (isEmptyFileInfo(fileInfoBuffer) && fileInfoBuffer->keepCopying)
    {
        pthread_cond_wait(&fileInfoBuffer->not_empty, &fileInfoBuffer->mutex);
    }
    if (isEmptyFileInfo(fileInfoBuffer))
    {
        pthread_mutex_unlock(&fileInfoBuffer->mutex);
        return NULL;
    }
    FileInfo *fileInfo = &fileInfoBuffer->buffer[fileInfoBuffer->readIndex];
    fileInfoBuffer->readIndex = (fileInfoBuffer->readIndex + 1) % BUFFER_SIZE;
    pthread_cond_broadcast(&fileInfoBuffer->not_full);
    pthread_mutex_unlock(&fileInfoBuffer->mutex);
    return fileInfo;
}

int isEmptyFileInfo(FileInfoBuffer *fileInfoBuffer)
{
    return fileInfoBuffer->writeIndex == fileInfoBuffer->readIndex;
}

int isFullFileInfo(FileInfoBuffer *fileInfoBuffer)
{
    return ((fileInfoBuffer->writeIndex + 1) % BUFFER_SIZE) == fileInfoBuffer->readIndex;
}

void startCopying(FileInfoBuffer *fileInfoBuffer)
{
    printf("Starting copying...\n");
    fileInfoBuffer->keepCopying = 1;
}

void stopCopying(FileInfoBuffer *fileInfoBuffer)
{
    pthread_mutex_lock(&fileInfoBuffer->mutex);
    pthread_cond_broadcast(&fileInfoBuffer->not_empty);
    fileInfoBuffer->keepCopying = 0;
    pthread_mutex_unlock(&fileInfoBuffer->mutex);
}