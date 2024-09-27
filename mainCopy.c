#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>

#define MAX_NAME_LENGTH 100
#define NUM_THREADS 3
#define BUFFER_SIZE 10

// Variable to control the threads execution
// 1: Keep copying
// 0: Stop copying
// it will be set by the main thread once it processes all the files to fill in the buffer
int keepCopying = 1;

// BUFFERS
FileInfoBuffer *FILE_INFO_BUFFER;
LogInfoBuffer *LOG_INFO_BUFFER;

/*
FileInfo stores the information of a file to be copied
It has the origin path, the destination path and the size in bytes
*/
typedef struct
{
    char origin[MAX_NAME_LENGTH];      // Field to store the origin path
    char destination[MAX_NAME_LENGTH]; // Field to store the destination path
    size_t size;                       // Field to store the size in bytes
} FileInfo;

/*
readDirectory reads the files in a directory and stores the information in the FILE_INFO_BUFFER
*/
void readDirectory(const char *sourceDir, const char *destDir)
{
    DIR *dir;
    struct dirent *entry; // Entries in the directory, files or subdirectories
    struct stat statbuf;  // Struct to store file information, like size

    dir = opendir(sourceDir);

    if (dir == NULL)
    {
        perror("Error opening directory");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL)
    { 
        char sourcePath[MAX_NAME_LENGTH];
        char destPath[MAX_NAME_LENGTH];

        // Skip the current directory and the parent directory
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        // Create the full path for the source and destination files
        snprintf(sourcePath, sizeof(sourcePath), "%s/%s", sourceDir, entry->d_name);
        snprintf(destPath, sizeof(destPath), "%s/%s", destDir, entry->d_name);

        // Fill the buffer with the file information
        if (stat(sourcePath, &statbuf) == 0)
        {
            // Validate if the entry is a file
            if (S_ISREG(statbuf.st_mode))
            {
                FileInfo *fileInfo = malloc(sizeof(FileInfo)); // Memory allocation for the struct
                strcpy(fileInfo->origin, sourcePath);
                strcpy(fileInfo->destination, destPath);
                fileInfo->size = statbuf.st_size;

                writeFileInfo(FILE_INFO_BUFFER, fileInfo);
            }
            // Validate if the entry is a directory
            else if (S_ISDIR(statbuf.st_mode))
            {
                // Create the directory in the destination path
                mkdir(destPath, 0755);

                // Recursive call to read the directory
                readDirectory(sourcePath, destPath);
            }
        }
    }
    closedir(dir); 
}


/*
LogInfo stores the information of a file that was copied
It has the name, the size in bytes and the duration in miliseconds
*/
typedef struct
{
    char name[MAX_NAME_LENGTH]; // Field to store the name
    size_t size;                // In bytes
    double duration;            // In miliseconds
} LogInfo;

LogInfo *newLogInfo(/* char *name, size_t size, time_t timestamp */)
{
    LogInfo *fileInfo = malloc(sizeof(LogInfo));
    /* snprintf(fileInfo->name, MAX_NAME_LENGTH, "%s", name);
    fileInfo->size = size;
    fileInfo->timestamp = timestamp; */
    return fileInfo;
}

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

/*
newLogInfoBuffer creates a new FileInfoBuffer struct and initializes it
*/
LogInfoBuffer *newLogInfoBuffer()
{
    LogInfoBuffer *logInfoBuffer = malloc(sizeof(FileInfoBuffer));
    pthread_mutex_init(&logInfoBuffer->mutex, NULL);
    pthread_cond_init(&logInfoBuffer->not_full, NULL);
    pthread_cond_init(&logInfoBuffer->not_empty, NULL);
    logInfoBuffer->buffer = malloc(BUFFER_SIZE * sizeof(FileInfo));
    logInfoBuffer->readIndex = 0;
    logInfoBuffer->writeIndex = 0;
    return logInfoBuffer;
}

/*
freeFileInfoBuffer frees the memory allocated for the FileInfoBuffer struct
*/
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

/*
readFileInfo reads a FileInfo struct from the buffer
*/
LogInfo *readFileInfo(LogInfoBuffer *logInfoBuffer)
{
    pthread_mutex_lock(&logInfoBuffer->mutex); // Lock the mutex
    while (logInfoBuffer->writeIndex == logInfoBuffer->readIndex)
    {
        pthread_cond_wait(&logInfoBuffer->not_empty, &logInfoBuffer->mutex);
    }
    LogInfo *logInfo = &logInfoBuffer->buffer[logInfoBuffer->readIndex];     // Read the struct from the buffer
    logInfoBuffer->readIndex = (logInfoBuffer->readIndex + 1) % BUFFER_SIZE; // Increase the read index, and uses the modulo operator to keep it in the range [0, BUFFER_SIZE)

    pthread_cond_signal(&logInfoBuffer->not_full); // Signal that a new item has been read

    pthread_mutex_unlock(&logInfoBuffer->mutex); // Unlock the mutex
    return logInfo;
}

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
    FileInfo *buffer; // Dynamic buffer for strings
    int readIndex;
    int writeIndex;

} FileInfoBuffer;

FileInfoBuffer *newFileNameBuffer()
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

void freeFFileInfoBuffer(FileInfoBuffer *fileInfoBuffer)
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

void *copyFiles(void *arg)
{
    int threadNum = *((int *)arg);

    while (keepCopying)
    {
        if (FILE_INFO_BUFFER == NULL)
        {
            printf("Error: fileNameBuffer is null. Thread '#%ld' is going to stop.\n", threadNum);
            return;
        }

        LogInfo *logInfo = newLogInfo();

        FileInfo *fileInfo = readFileName(FILE_INFO_BUFFER);
        // !!! i am not using size

        snprintf(logInfo->name, MAX_NAME_LENGTH, "%s", fileInfo->destination);

        // Copy the file
        int sourceFD = open(fileInfo->origin, O_RDONLY);
        if (sourceFD == -1)
        {
            printf("Error: opening source file '%s' on thread '#%ld'.\n", fileInfo->origin, threadNum);
            continue;
        }

        // ??? I THINK THIS DESTINATION SHOULD CHANGE
        int destFD = open(fileInfo->destination, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (destFD == -1)
        {
            perror("Error opening destination file"); // TODO : extend print
            close(sourceFD);
            continue;
        }

        char buffer[1024];
        ssize_t bytesRead;
        clock_t start = clock();
        while ((bytesRead = read(sourceFD, buffer, sizeof(buffer))) > 0)
        {
            fileInfo->size += bytesRead;
            if (write(destFD, buffer, bytesRead) != bytesRead)
            {
                perror("Error writing to destination file"); // TODO : extend print
                close(sourceFD);
                close(destFD);
                break;
            }
        }
        clock_t end = clock();

        logInfo->duration = (double)(end - start) * 1000 / CLOCKS_PER_SEC;

        close(sourceFD);
        close(destFD);

        // Lock the mutex to increment the filesCopied counter
        writeLogInfo(LOG_INFO_BUFFER, logInfo);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <source_dir> <dest_dir>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *sourceDir = argv[1]; // Source directory where are contained the files to copy
    const char *destDir = argv[2];   // Destination directory where the files will be copied

    readDirectory(sourceDir, destDir);

    // Create threads
    pthread_t threads[NUM_THREADS];
    int threadIds[NUM_THREADS]; // Array to store the thread ids, gives a number to each thread to identify them

    for (int i = 0; i < NUM_THREADS; i++)
    {
        threadIds[i] = i;
        pthread_create(&threads[i], NULL, copyFiles, &threadIds[i]); // Missing the copyFiles function
    }

    // Main thread waits for his son threads to finish, before he continues
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return 0;
}