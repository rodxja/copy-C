#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "loginfo.h"

// Test for creating a new LogInfo object
void test_newLogInfo()
{
    LogInfo *logInfo = newLogInfo();

    assert(logInfo != NULL);                           // Ensure the logInfo object is created
    assert(strcmp(logInfo->name, "default_log") == 0); // Check default name
    assert(logInfo->size == 0);                        // Check default size
    assert(logInfo->duration == 0);                    // Check timestamp is set

    free(logInfo); // Free memory after the test
    printf("test_newLogInfo passed\n");
}

// Test for creating and freeing a LogInfoBuffer
void test_newLogInfoBuffer()
{
    LogInfoBuffer *logBuffer = newLogInfoBuffer();

    assert(logBuffer != NULL);          // Ensure the buffer is created
    assert(logBuffer->readIndex == 0);  // Initial read index should be 0
    assert(logBuffer->writeIndex == 0); // Initial write index should be 0
    assert(logBuffer->buffer != NULL);  // Ensure the buffer array is initialized

    freeLogInfoBuffer(logBuffer); // Free memory after the test
    printf("test_newLogInfoBuffer passed\n");
}

// Test for writing to the buffer
void test_writeLogInfo()
{
    LogInfoBuffer *logBuffer = newLogInfoBuffer();
    LogInfo *logInfo = newLogInfo();

    snprintf(logInfo->name, sizeof(logInfo->name), "testfile.txt");
    logInfo->size = 2048;
    logInfo->duration = 0;

    writeLogInfo(logBuffer, logInfo); // Write to the buffer

    assert(strcmp(logBuffer->buffer[logBuffer->readIndex].name, "testfile.txt") == 0); // Check if written correctly
    assert(logBuffer->buffer[logBuffer->readIndex].size == 2048);
    assert(logBuffer->buffer[logBuffer->readIndex].duration == logInfo->duration);

    freeLogInfoBuffer(logBuffer);
    free(logInfo);
    printf("test_writeLogInfo passed\n");
}

// Test for reading from the buffer
void test_readLogInfo()
{
    LogInfoBuffer *logBuffer = newLogInfoBuffer();
    LogInfo *logInfo = newLogInfo();

    snprintf(logInfo->name, sizeof(logInfo->name), "testfile.txt");
    logInfo->size = 1024;
    logInfo->duration = 0;

    writeLogInfo(logBuffer, logInfo); // Write to the buffer

    LogInfo *readLog = readLogInfo(logBuffer); // Read from the buffer

    assert(strcmp(readLog->name, "testfile.txt") == 0); // Check if read correctly
    assert(readLog->size == 1024);
    assert(readLog->duration == logInfo->duration);

    freeLogInfoBuffer(logBuffer);
    free(logInfo);
    printf("test_readLogInfo passed\n");
}

int main()
{
    // Call the test functions
    test_newLogInfo();
    test_newLogInfoBuffer();
    test_writeLogInfo();
    test_readLogInfo();

    printf("All tests passed!\n");
    return 0;
}

// gcc -o test_loginfo test_loginfo.c loginfo.c -lpthread
