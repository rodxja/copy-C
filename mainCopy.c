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
    char origin[MAX_NAME_LENGTH];  
    char destination[MAX_NAME_LENGTH];  
    size_t size;                 
} FileInfo;

pthread_mutex_t fileInfoMutex = PTHREAD_MUTEX_INITIALIZER;
FileInfo* filesBuffer;
int fileInfoCount = 0;
int filesCopied = 0;
int FileIndex = 0;

void readDirectory(const char* sourceDir, const char* destDir) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;

    dir = opendir(sourceDir);

    if (dir == NULL) {
        perror("Error opening directory");
        exit(EXIT_FAILURE);
    }

    filesBuffer = malloc(100 * sizeof(FileInfo));

    while ((entry = readdir(dir)) != NULL) {
        char sourcePath[MAX_NAME_LENGTH];
        char destPath[MAX_NAME_LENGTH];

        snprintf(sourcePath, sizeof(sourcePath), "%s/%s", sourceDir, entry->d_name);
        snprintf(destPath, sizeof(destPath), "%s/%s", destDir, entry->d_name);

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

    const char* sourceDir = argv[1];
    const char* destDir = argv[2];

    readDirectory(sourceDir, destDir);

    // Create threads
}