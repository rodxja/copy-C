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

typedef struct {
    char origin[MAX_NAME_LENGTH];  // Field to store the origin path
    char destination[MAX_NAME_LENGTH];  // Field to store the destination path
    size_t size;                        // Field to store the size in bytes         
} FileInfo;

pthread_mutex_t fileInfoMutex = PTHREAD_MUTEX_INITIALIZER;
FileInfo* filesBuffer;
int fileInfoCount = 0;
int filesCopied = 0;
int FileIndex = 0;      // Index to keep track of the file being copied, 
                        // important so that the threads don't copy the same file

void readDirectory(const char* sourceDir, const char* destDir) {
    DIR *dir;
    struct dirent *entry; //Entries in the directory, files or subdirectories
    struct stat statbuf;  //Struct to store file information, like size

    dir = opendir(sourceDir);

    if (dir == NULL) { 
        perror("Error opening directory");
        exit(EXIT_FAILURE);
    }

    filesBuffer = malloc(100 * sizeof(FileInfo));

    while ((entry = readdir(dir)) != NULL) { //Iterate over the entries in the directory
        char sourcePath[MAX_NAME_LENGTH];
        char destPath[MAX_NAME_LENGTH];

        // Create the full path for the source and destination files
        snprintf(sourcePath, sizeof(sourcePath), "%s/%s", sourceDir, entry->d_name); 
        snprintf(destPath, sizeof(destPath), "%s/%s", destDir, entry->d_name);

        // Fill the buffer with the file information
        if (stat(sourcePath, &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
            strcpy(filesBuffer[fileInfoCount].origin, sourcePath);
            strcpy(filesBuffer[fileInfoCount].destination, destPath);
            filesBuffer[fileInfoCount].size = statbuf.st_size;
            fileInfoCount++;
        }
    }

    closedir(dir);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_dir> <dest_dir>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* sourceDir = argv[1]; // Source directory where are contained the files to copy
    const char* destDir = argv[2]; // Destination directory where the files will be copied

    readDirectory(sourceDir, destDir);

    // Create threads
    pthread_t threads[NUM_THREADS];
    int threadIds[NUM_THREADS]; //Array to store the thread ids, gives a number to each thread to identify them

    for (int i = 0; i < NUM_THREADS; i++) {
        threadIds[i] = i;
        pthread_create(&threads[i], NULL, copyFiles, &threadIds[i]); //Missing the copyFiles function
    }

    // Main thread waits for his son threads to finish, before he continues
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Files copied: %d\n", filesCopied);

    free(filesBuffer);

    return 0;
}