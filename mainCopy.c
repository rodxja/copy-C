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
FileInfoBuffer *fileInfoBuffer;
FileNameBuffer *fileNameBuffer;

char *destinationDir;

typedef struct
{
    char origin[MAX_NAME_LENGTH];      // Field to store the origin path
    char destination[MAX_NAME_LENGTH]; // Field to store the destination path
    size_t size;                       // Field to store the size in bytes
} FileInfo;

pthread_mutex_t fileInfoMutex = PTHREAD_MUTEX_INITIALIZER;
FileInfo *filesBuffer;
int fileInfoCount = 0;
int filesCopied = 0;
int FileIndex = 0; // Index to keep track of the file being copied,
                   // important so that the threads don't copy the same file

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

    filesBuffer = malloc(100 * sizeof(FileInfo));

    while ((entry = readdir(dir)) != NULL)
    { // Iterate over the entries in the directory
        char sourcePath[MAX_NAME_LENGTH];
        char destPath[MAX_NAME_LENGTH];

        // Create the full path for the source and destination files
        snprintf(sourcePath, sizeof(sourcePath), "%s/%s", sourceDir, entry->d_name);
        snprintf(destPath, sizeof(destPath), "%s/%s", destDir, entry->d_name);

        // Fill the buffer with the file information
        if (stat(sourcePath, &statbuf) == 0 && S_ISREG(statbuf.st_mode))
        {
            strcpy(filesBuffer[fileInfoCount].origin, sourcePath);
            strcpy(filesBuffer[fileInfoCount].destination, destPath);
            filesBuffer[fileInfoCount].size = statbuf.st_size;
            fileInfoCount++;
        }
    }

    closedir(dir);
}

typedef struct
{
    char name[MAX_NAME_LENGTH]; // Field to store the name
    size_t size;                // In bytes
    double duration;            // In miliseconds
} FileInfo;

FileInfo *newFileInfo(/* char *name, size_t size, time_t timestamp */)
{
    FileInfo *fileInfo = malloc(sizeof(FileInfo));
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
    FileInfo *buffer; // Dynamic buffer for structs
    int readIndex;
    int writeIndex;

} FileInfoBuffer;

// TODO : Implement the following in BOTH FileInfoBuffer and FileNameBuffer
// 1. CONTROL THE ACCESS TO THE BUFFER SO AS NOT TO WRITE OVER A FILE INTO THAT WAS NOT READ YET
// 2. CONTROL THE WRITE TO WAIT UNTIL THERE IS SPACE IN THE BUFFER

/*
newFileInfoBuffer creates a new FileInfoBuffer struct and initializes it
*/
FileInfoBuffer *newFileInfoBuffer()
{
    FileInfoBuffer *buffer = malloc(sizeof(FileInfoBuffer));
    pthread_mutex_init(&buffer->mutex, NULL);
    buffer->buffer = malloc(BUFFER_SIZE * sizeof(FileInfo));
    buffer->readIndex = 0;
    buffer->writeIndex = 0;
    return buffer;
}

/*
freeFileInfoBuffer frees the memory allocated for the FileInfoBuffer struct
*/
void freeFileInfoBuffer(FileInfoBuffer *buffer)
{
    free(buffer->buffer);
    free(buffer);
}

/*
writeFileInfo writes a FileInfo struct to the buffer
*/
void writeFileInfo(FileInfoBuffer *buffer, FileInfo *fileInfo)
{
    pthread_mutex_lock(&buffer->mutex);                          // Lock the mutex
    buffer->buffer[buffer->writeIndex] = *fileInfo;              // Write the struct to the buffer
    buffer->writeIndex = (buffer->writeIndex + 1) % BUFFER_SIZE; // Increase the write index, and uses the modulo operator to keep it in the range [0, BUFFER_SIZE)
    pthread_mutex_unlock(&buffer->mutex);                        // Unlock the mutex
}

/*
readFileInfo reads a FileInfo struct from the buffer
*/
FileInfo *readFileInfo(FileInfoBuffer *buffer)
{
    pthread_mutex_lock(&buffer->mutex);                        // Lock the mutex
    FileInfo *fileInfo = &buffer->buffer[buffer->readIndex];   // Read the struct from the buffer
    buffer->readIndex = (buffer->readIndex + 1) % BUFFER_SIZE; // Increase the read index, and uses the modulo operator to keep it in the range [0, BUFFER_SIZE)
    pthread_mutex_unlock(&buffer->mutex);                      // Unlock the mutex
    return fileInfo;
}

typedef struct
{
    pthread_mutex_t mutex;
    char **stringsBuffer; // Dynamic buffer for strings
    int readIndex;
    int writeIndex;

} FileNameBuffer;

FileNameBuffer *newFileNameBuffer()
{
    FileNameBuffer *buffer = malloc(sizeof(FileNameBuffer));
    pthread_mutex_init(&buffer->mutex, NULL);
    buffer->stringsBuffer = malloc(BUFFER_SIZE * sizeof(char *));
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        buffer->stringsBuffer[i] = malloc(MAX_NAME_LENGTH * sizeof(char));
    }
    buffer->readIndex = 0;
    buffer->writeIndex = 0;
    return buffer;
}

void freeFileNameBuffer(FileNameBuffer *buffer)
{
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        free(buffer->stringsBuffer[i]);
    }
    free(buffer->stringsBuffer);
    free(buffer);
}

void writeFileName(FileNameBuffer *buffer, char *fileName)
{
    pthread_mutex_lock(&buffer->mutex);
    snprintf(buffer->stringsBuffer[buffer->writeIndex], MAX_NAME_LENGTH, "%s", fileName);
    buffer->writeIndex = (buffer->writeIndex + 1) % BUFFER_SIZE;
    pthread_mutex_unlock(&buffer->mutex);
}

char *readFileName(FileNameBuffer *buffer)
{
    pthread_mutex_lock(&buffer->mutex);
    char *fileName = buffer->stringsBuffer[buffer->readIndex];
    buffer->readIndex = (buffer->readIndex + 1) % BUFFER_SIZE;
    pthread_mutex_unlock(&buffer->mutex);
    return fileName;
}

void *copyFiles(void *arg)
{
    int threadNum = *((int *)arg);

    while (keepCopying)
    {
        if (fileNameBuffer == NULL)
        {
            printf("Error: fileNameBuffer is null. Thread '#%ld' is going to stop.\n", threadNum);
            return;
        }

        FileInfo *fileInfo = newFileInfo();

        char *fileName = readFileName(fileNameBuffer);

        snprintf(fileInfo->name, MAX_NAME_LENGTH, "%s", fileName);

        // Copy the file
        int sourceFD = open(fileName, O_RDONLY);
        if (sourceFD == -1)
        {
            printf("Error: opening source file '%s' on thread '#%ld'.\n", fileName, threadNum);
            continue;
        }

        // ??? I THINK THIS DESTINATION SHOULD CHANGE
        int destFD = open(destinationDir, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (destFD == -1)
        {
            perror("Error opening destination file");
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
                perror("Error writing to destination file");
                close(sourceFD);
                close(destFD);
                break;
            }
        }
        clock_t end = clock();

        fileInfo->duration = (double)(end - start) * 1000 / CLOCKS_PER_SEC;

        close(sourceFD);
        close(destFD);

        // Lock the mutex to increment the filesCopied counter
        writeFileInfo(fileInfoBuffer, fileInfo);
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

    destinationDir = destDir;

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

    printf("Files copied: %d\n", filesCopied);

    free(filesBuffer);

    return 0;
}