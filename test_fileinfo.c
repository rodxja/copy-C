#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "fileinfo.h"


// Test for newFileInfoBuffer
void test_newFileInfoBuffer()
{
    FileInfoBuffer *buffer = newFileInfoBuffer();
    assert(buffer != NULL);
    assert(buffer->readIndex == 0);
    assert(buffer->writeIndex == 0);
    assert(buffer->buffer != NULL);

    freeFileInfoBuffer(buffer);
    printf("test_newFileInfoBuffer passed!\n");
}

// Test for writeFileInfo and readFileInfo
void test_writeReadFileInfo()
{
    FileInfoBuffer *buffer = newFileInfoBuffer();
    FileInfo fileInfo;

    // Initialize FileInfo
    snprintf(fileInfo.origin, MAX_NAME_LENGTH, "file1.txt");
    snprintf(fileInfo.destination, MAX_NAME_LENGTH, "file1_copy.txt");
    fileInfo.size = 1024;

    // Write FileInfo to the buffer
    writeFileInfo(buffer, &fileInfo);

    // Read FileInfo from the buffer
    FileInfo *retrievedFileInfo = readFileInfo(buffer);

    // Validate the contents of the struct
    assert(strcmp(retrievedFileInfo->origin, "file1.txt") == 0);
    assert(strcmp(retrievedFileInfo->destination, "file1_copy.txt") == 0);
    assert(retrievedFileInfo->size == 1024);

    freeFileInfoBuffer(buffer);
    printf("test_writeReadFileInfo passed!\n");
}

// Test for buffer overflow handling
void test_bufferOverflow()
{
    FileInfoBuffer *buffer = newFileInfoBuffer();
    FileInfo fileInfo;

    // Fill the buffer
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        snprintf(fileInfo.origin, MAX_NAME_LENGTH, "file%d.txt", i);
        snprintf(fileInfo.destination, MAX_NAME_LENGTH, "file%d_copy.txt", i);
        fileInfo.size = i * 1024;

        writeFileInfo(buffer, &fileInfo);
    }

    // Try to write one more, it should wait/block
    snprintf(fileInfo.origin, MAX_NAME_LENGTH, "file_overflow.txt");
    snprintf(fileInfo.destination, MAX_NAME_LENGTH, "file_overflow_copy.txt");
    fileInfo.size = 9999;

    // This should still work because of our circular buffer
    writeFileInfo(buffer, &fileInfo);

    FileInfo *retrievedFileInfo = readFileInfo(buffer);
    assert(strcmp(retrievedFileInfo->origin, "file0.txt") == 0); // The first file should be the first read

    freeFileInfoBuffer(buffer);
    printf("test_bufferOverflow passed!\n");
}

// Test for buffer underflow handling
void test_bufferUnderflow()
{
    FileInfoBuffer *buffer = newFileInfoBuffer();

    // Try to read without writing, this should block/wait
    FileInfo *retrievedFileInfo = readFileInfo(buffer); // Should wait on condition

    // However, as it's a unit test, we avoid infinite blocking and handle it manually for now
    assert(retrievedFileInfo == NULL); // Should be NULL since nothing was written

    freeFileInfoBuffer(buffer);
    printf("test_bufferUnderflow passed!\n");
}

int main()
{
    test_newFileInfoBuffer();
    test_writeReadFileInfo();
    test_bufferOverflow();
    test_bufferUnderflow();

    printf("All tests passed!\n");
    return 0;
}
// gcc -o test_fileinfo test_fileinfo.c fileinfo.c -pthread