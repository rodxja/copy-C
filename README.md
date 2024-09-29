# Copy

command <origin_directory> <destination_directoty>

origin_directory must exist and contain files
destination_directory must exist and not include several folder
    correct : ./dest
    incorrect : ./dest/sub



it works as expected when buffer is greater than items to copy
it works as expected when num of threads is less than items to copy


pending to fix
1. when buffer is less than items to copy
        so thread is waiting to write
        this happens then readDirectory is blocked due to no space in buffer to write
        so it waits until other thread to read from it in order to have an empty space
        but as readDirectory is not a thread it will block the whole program
    how to fix it:
    handle readDirectory as thread
2. when num of threads is greater than items to copy
    i suppose that threads are waiting to read so they did not finished
    probably check read and write functions

tests:
    buffer of one item and copy n
    more threads than items to copy


new error 
    LogInfoBuffer is locked when keepLogging in theory is false and when it has not logInfo
    it is replicated at first run after build
    * see the following code
     while (!hasLogInfo(logInfoBuffer) && keepLogging)
    {
        printf("LogInfo Buffer is empty, waiting for a write\n");
        pthread_cond_wait(&logInfoBuffer->not_empty, &logInfoBuffer->mutex);
    }
    i am assuming that while (!hasLogInfo(logInfoBuffer) && keepLogging) will be check at some time after wait
    but pthread_cond_wait is waiting for logInfoBuffer->not_empty to be change in order to stop blocking

    yes, it was for that, now there is no deadlock due that stopKeep... functions are sending a signal and unblocking the thread

new issue
    reading FileInfo 'FileInfo: (null) -> (null). Size 0.' from buffer[4]
    Segmentation fault (core dumped)

    after unblocking it is reading an empty item
    it should not be read

    validate that there is data and return null