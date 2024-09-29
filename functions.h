#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fileinfo.h"
#include "loginfo.h"

extern int keepCopying;
extern int keepLogging;

// BUFFERS
extern FileInfoBuffer *FILE_INFO_BUFFER;
extern LogInfoBuffer *LOG_INFO_BUFFER;

// Function prototypes
void readDirectory(const char *sourceDir, const char *destDir);
void *copy(void *arg);
void *writeLog(void *arg);

#endif // FUNCTIONS_H
