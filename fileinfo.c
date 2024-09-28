#include "fileinfo.h"
#include <stdio.h>
#include <string.h>

FileInfo *newFileInfo()
{
    FileInfo *fileInfo = malloc(sizeof(FileInfo));
    fileInfo->size = 0;
    return fileInfo;
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
    while (((fileInfoBuffer->writeIndex + 1) % BUFFER_SIZE) == fileInfoBuffer->readIndex)
    {
        pthread_cond_wait(&fileInfoBuffer->not_full, &fileInfoBuffer->mutex);
    }
    fileInfoBuffer->buffer[fileInfoBuffer->writeIndex] = *fileInfo;
    fileInfoBuffer->writeIndex = (fileInfoBuffer->writeIndex + 1) % BUFFER_SIZE;
    pthread_cond_signal(&fileInfoBuffer->not_empty);
    pthread_mutex_unlock(&fileInfoBuffer->mutex);
}

FileInfo *readFileInfo(FileInfoBuffer *fileInfoBuffer)
{
    pthread_mutex_lock(&fileInfoBuffer->mutex);
    while (fileInfoBuffer->writeIndex == fileInfoBuffer->readIndex)
    {
        pthread_cond_wait(&fileInfoBuffer->not_empty, &fileInfoBuffer->mutex);
    }
    FileInfo *fileInfo = &fileInfoBuffer->buffer[fileInfoBuffer->readIndex];
    fileInfoBuffer->readIndex = (fileInfoBuffer->readIndex + 1) % BUFFER_SIZE;
    pthread_cond_signal(&fileInfoBuffer->not_full);
    pthread_mutex_unlock(&fileInfoBuffer->mutex);
    return fileInfo;
}
