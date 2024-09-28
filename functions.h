#ifndef FUNCTIONS_H
#define FUNCTIONS_H

extern int keepCopying;

// BUFFERS
extern FileInfoBuffer *FILE_INFO_BUFFER;
extern LogInfoBuffer *LOG_INFO_BUFFER;

// Function prototypes
void *copy(void *arg);

#endif // FUNCTIONS_H
