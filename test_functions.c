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

    // Crear directorios de prueba
    mkdir(sourceDir, 0700);
    mkdir(sourceSubDir, 0700);

    // Crear archivos de prueba en el directorio origen
    char filePath1[MAX_NAME_LENGTH];
    char filePath2[MAX_NAME_LENGTH];
    char fileSubPath3[MAX_NAME_LENGTH];
    snprintf(filePath1, sizeof(filePath1), "%s/%s", sourceDir, "testfile1.txt");
    snprintf(filePath2, sizeof(filePath2), "%s/%s", sourceDir, "testfile2.txt");
    snprintf(fileSubPath3, sizeof(fileSubPath3), "%s/%s", sourceSubDir, "testfile3.txt");

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

    // Crear directorios de prueba
    mkdir(sourceDir, 0700);
    mkdir(sourceSubDir, 0700);

    // Crear archivos de prueba en el directorio origen
    char filePath1[MAX_NAME_LENGTH];
    char filePath2[MAX_NAME_LENGTH];
    char fileSubPath3[MAX_NAME_LENGTH];
    snprintf(filePath1, sizeof(filePath1), "%s/%s", sourceDir, "testfile1.txt");
    snprintf(filePath2, sizeof(filePath2), "%s/%s", sourceDir, "testfile2.txt");
    snprintf(fileSubPath3, sizeof(fileSubPath3), "%s/%s", sourceSubDir, "testfile3.txt");

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

    // llamar a un solo copy
    int threadNum = 1;
    printf("Calling copy...\n");
    copy(&threadNum);

    printf("Copy finished.\n");

    // los archivos deberian ser copiados exitosamente

    unlink(filePath1);
    unlink(filePath2);
    unlink(fileSubPath3);

    // ahora verificar que los datos están en LOG_INFO_BUFFER

    LogInfo *logInfo1 = readLogInfo(LOG_INFO_BUFFER);
    LogInfo *logInfo2 = readLogInfo(LOG_INFO_BUFFER);
    LogInfo *logInfo3 = readLogInfo(LOG_INFO_BUFFER);

    printf("LogInfo1: %s. Size %d.\n", logInfo1->name, logInfo1->size);
    printf("LogInfo2: %s. Size %d.\n", logInfo2->name, logInfo2->size);
    printf("LogInfo3: %s. Size %d.\n", logInfo3->name, logInfo3->size);

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