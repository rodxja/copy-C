#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fileinfo.h"

extern FileInfoBuffer *FILE_INFO_BUFFER;

void readDirectory(const char *sourceDir, const char *destDir);

#endif // READINFO_H