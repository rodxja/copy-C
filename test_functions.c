#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include "functions.h"
#include "fileinfo.h"

// Unit Test para la función readDirectory
void test_readDirectory()
{
    // Inicializar buffers
    FILE_INFO_BUFFER = newFileInfoBuffer();

    const char *sourceDir = "./test_src_dir";
    const char *sourceSubDir = "./test_src_dir/subdir";
    const char *destDir = "./test_dest_dir";

    // files name
    const char *fileName1 = "testfile1.txt";
    const char *fileName2 = "testfile2.txt";
    const char *fileName3 = "testfile3.txt";

    // Crear directorios de prueba
    mkdir(sourceDir, 0700);
    mkdir(sourceSubDir, 0700);

    // Crear archivos de prueba en el directorio origen
    size_t len1 = strlen(sourceDir) + strlen(fileName1) + 2;    // +2 for the '/' and the null terminator
    char *filePath1 = malloc(len1);                             // +2 for the '/' and the null terminator
    size_t len2 = strlen(sourceDir) + strlen(fileName2) + 2;    // +2 for the '/' and the null terminator
    char *filePath2 = malloc(len2);                             // +2 for the '/' and the null terminator
    size_t len3 = strlen(sourceSubDir) + strlen(fileName3) + 2; // +2 for the '/' and the null terminator
    char *fileSubPath3 = malloc(len3);

    snprintf(filePath1, len1, "%s/%s", sourceDir, fileName1);
    snprintf(filePath2, len2, "%s/%s", sourceDir, fileName2);
    snprintf(fileSubPath3, len3, "%s/%s", sourceSubDir, fileName3);

    // Crear archivos de prueba y escribir algunos datos en ellos
    int fd1 = open(filePath1, O_CREAT | O_WRONLY, 0644);
    write(fd1, "Hello, World!", 13); // Escribir 13 bytes
    close(fd1);

    int fd2 = open(filePath2, O_CREAT | O_WRONLY, 0644);
    write(fd2, "Test file content", 17); // Escribir 17 bytes
    close(fd2);

    int fd3 = open(fileSubPath3, O_CREAT | O_WRONLY, 0644);
    write(fd3, "Subdir file content", 19);
    close(fd3);

    // Llamar a la función a probar
    readDirectory(sourceDir, destDir);

    FileInfo *fileInfo1 = readFileInfo(FILE_INFO_BUFFER);
    printf("Origin: %s\n", fileInfo1->origin);

    FileInfo *fileInfo2 = readFileInfo(FILE_INFO_BUFFER);
    printf("Origin: %s\n", fileInfo2->origin);

    FileInfo *fileInfo3 = readFileInfo(FILE_INFO_BUFFER);
    printf("Origin: %s\n", fileInfo3->origin);

    // Limpiar archivos y directorios de prueba
    unlink(filePath1);
    unlink(filePath2);
    unlink(fileSubPath3);

    printf("Removing directories...\n");

    printf("Removing directory %s\n", sourceSubDir);
    rmdir(sourceSubDir);

    printf("Removing directory %s\n", sourceDir);
    rmdir(sourceDir);

    // Liberar memoria del buffer
    free(FILE_INFO_BUFFER);
}

void test_fullcopy()
{
    // Inicializar buffers
    FILE_INFO_BUFFER = newFileInfoBuffer();
    LOG_INFO_BUFFER = newLogInfoBuffer();

    const char *sourceDir = "./test_src_dir";
    const char *sourceSubDir = "./test_src_dir/subdir";
    const char *destDir = "./test_dest_dir";

    // files name
    const char *fileName1 = "testfile1.txt";
    const char *fileName2 = "testfile2.txt";
    const char *fileName3 = "testfile3.txt";

    // Crear directorios de prueba
    mkdir(sourceDir, 0700);
    mkdir(sourceSubDir, 0700);

    // Crear archivos de prueba en el directorio origen
    size_t len1 = strlen(sourceDir) + strlen(fileName1) + 2;    // +2 for the '/' and the null terminator
    char *filePath1 = malloc(len1);                             // +2 for the '/' and the null terminator
    size_t len2 = strlen(sourceDir) + strlen(fileName2) + 2;    // +2 for the '/' and the null terminator
    char *filePath2 = malloc(len2);                             // +2 for the '/' and the null terminator
    size_t len3 = strlen(sourceSubDir) + strlen(fileName3) + 2; // +2 for the '/' and the null terminator
    char *fileSubPath3 = malloc(len3);

    snprintf(filePath1, len1, "%s/%s", sourceDir, fileName1);
    snprintf(filePath2, len2, "%s/%s", sourceDir, fileName2);
    snprintf(fileSubPath3, len3, "%s/%s", sourceSubDir, fileName3);

    // Crear archivos de prueba y escribir algunos datos en ellos
    int fd1 = open(filePath1, O_CREAT | O_WRONLY, 0644);
    write(fd1, "Hello, World!", 13); // Escribir 13 bytes
    close(fd1);

    int fd2 = open(filePath2, O_CREAT | O_WRONLY, 0644);
    write(fd2, "Test file content", 17); // Escribir 17 bytes
    close(fd2);

    int fd3 = open(fileSubPath3, O_CREAT | O_WRONLY, 0644);
    write(fd3, "Subdir file content", 19);
    close(fd3);

    // fin de creación de archivos
    /*
    - test_src_dir
        - testfile1.txt
        - testfile2.txt
        - subdir
            - testfile3.txt
     */

    // Llamar a la función a probar
    readDirectory(sourceDir, destDir);

    for (int i = 0; i < 3; i++)
    {
        FileInfo fileInfo = FILE_INFO_BUFFER->buffer[i];
        printf("%s\n", toStringFileInfo(&fileInfo));
    }

    // llamar a un solo copy
    int threadNum = 1;
    printf("Calling copy...\n");
    copy(&threadNum);

    printf("Copy finished.\n");

    // los archivos deberian ser copiados exitosamente

    /* unlink(filePath1);
     unlink(filePath2);
     unlink(fileSubPath3);
     */

    // ahora verificar que los datos están en LOG_INFO_BUFFER

    for (int i = 0; i < 3; i++)
    {
        LogInfo logInfo = LOG_INFO_BUFFER->buffer[i];
        printf("%s\n", toStringLogInfo(&logInfo));
    }

    // Limpiar archivos y directorios de prueba

    free(FILE_INFO_BUFFER);
    free(LOG_INFO_BUFFER);
}

int main()
{
    // Ejecutar el test
    // test_readDirectory();
    test_fullcopy();

    printf("All tests passed!\n");
    return 0;
}

// gcc -o test_functions test_functions.c functions.c fileinfo.c loginfo.c -lpthread