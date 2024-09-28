#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "functions.h"

void test_copy()
{
    
}

int main()
{

    LOG_INFO_BUFFER = newLogInfoBuffer();
    // Call the test functions
    test_newLogInfo();
    test_newLogInfoBuffer();
    test_writeLogInfo();
    test_readLogInfo();

    printf("All tests passed!\n");
    return 0;
}
