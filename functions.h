#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fileinfo.h"

extern int keepCopying;

// BUFFERS
extern FileInfoBuffer *FILE_INFO_BUFFER;
extern LogInfoBuffer *LOG_INFO_BUFFER;

// Function prototypes
void *copy(void *arg);
void readDirectory(const char *sourceDir, const char *destDir);

#endif // FUNCTIONS_H
