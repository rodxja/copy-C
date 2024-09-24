/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define MAX_NAME_LENGTH 100
#define BUFFER_SIZE 10

typedef struct {
    char name[MAX_NAME_LENGTH];  // Field to store the name
    size_t size;                 // Field to store the size
    time_t timestamp;            // Field to store the time
} FileInfo;

pthread_mutex_t stringBufferMutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex for string buffer
pthread_mutex_t fileInfoBufferMutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex for struct buffer

char** stringsBuffer;          // Dynamic buffer for strings
FileInfo* fileInfoBuffer;      // Dynamic buffer for structs

void initializeBuffers() {
    // Initialize strings buffer
    stringsBuffer = malloc(BUFFER_SIZE * sizeof(char*));
    for (int i = 0; i < BUFFER_SIZE; i++) {
        stringsBuffer[i] = malloc(MAX_NAME_LENGTH * sizeof(char));  // Allocate memory for each string
    }
    
    // Initialize fileInfo buffer
    fileInfoBuffer = malloc(BUFFER_SIZE * sizeof(FileInfo));
}

void freeBuffers() {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        free(stringsBuffer[i]);  // Free each allocated string
    }
    free(stringsBuffer);  // Free the string buffer itself
    free(fileInfoBuffer);  // Free the struct buffer
}

// Example function to modify buffers
void* modifyBuffers(void* arg) {
    int threadNum = *((int*)arg);

    // Lock string buffer mutex
    pthread_mutex_lock(&stringBufferMutex);
    printf("Thread %d: Modifying string buffer...\n", threadNum);
    for (int i = 0; i < BUFFER_SIZE; i++) {
        snprintf(stringsBuffer[i], MAX_NAME_LENGTH, "Thread%d_File%d.txt", threadNum, i + 1);
    }
    pthread_mutex_unlock(&stringBufferMutex);  // Unlock string buffer mutex

    // Lock fileInfo buffer mutex
    pthread_mutex_lock(&fileInfoBufferMutex);
    printf("Thread %d: Modifying fileInfo buffer...\n", threadNum);
    for (int i = 0; i < BUFFER_SIZE; i++) {
        snprintf(fileInfoBuffer[i].name, MAX_NAME_LENGTH, "Thread%d_File%d.txt", threadNum, i + 1);
        fileInfoBuffer[i].size = 1024 * (i + 1);
        fileInfoBuffer[i].timestamp = time(NULL);
    }
    pthread_mutex_unlock(&fileInfoBufferMutex);  // Unlock fileInfo buffer mutex

    return NULL;
}

int main() {
    // Initialize buffers
    initializeBuffers();

    pthread_t threads[2];  // Example with 2 threads
    int threadNums[2] = {1, 2};

    // Create threads to modify the buffers
    for (int i = 0; i < 2; i++) {
        pthread_create(&threads[i], NULL, modifyBuffers, &threadNums[i]);
    }

    // Join threads
    for (int i = 0; i < 2; i++) {
        pthread_join(threads[i], NULL);
    }

    // Free the buffers
    freeBuffers();

    return 0;
}
*/