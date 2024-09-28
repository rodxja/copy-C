#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include "functions.h"
#include "fileinfo.h"


// Unit Test para la función readDirectory
void test_readDirectory() {
    const char *sourceDir = "./test_src_dir";
    const char *destDir = "./test_dest_dir";

    // Crear directorios de prueba
    mkdir(sourceDir, 0700);
    mkdir(destDir, 0700);

    // Crear archivos de prueba en el directorio origen
    char filePath1[MAX_NAME_LENGTH];
    char filePath2[MAX_NAME_LENGTH];
    snprintf(filePath1, sizeof(filePath1), "%s/%s", sourceDir, "testfile1.txt");
    snprintf(filePath2, sizeof(filePath2), "%s/%s", sourceDir, "testfile2.txt");

    // Crear archivos de prueba y escribir algunos datos en ellos
    int fd1 = open(filePath1, O_CREAT | O_WRONLY, 0644);
    write(fd1, "Hello, World!", 13);  // Escribir 13 bytes
    close(fd1);

    int fd2 = open(filePath2, O_CREAT | O_WRONLY, 0644);
    write(fd2, "Test file content", 17);  // Escribir 17 bytes
    close(fd2);

    // Crear subdifectorio en el directorio origen
    mkdir("./test_src_dir/subdir", 0700);

    char filePath3[MAX_NAME_LENGTH];
    snprintf(filePath3, sizeof(filePath3), "%s/%s", sourceDir, "subdir/testfile3.txt");

    int fd3 = open(filePath3, O_CREAT | O_WRONLY, 0644);
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
    rmdir(sourceDir);    
    rmdir(destDir);
}

int main() {
    // Inicializar buffers
    FILE_INFO_BUFFER = newFileInfoBuffer();

    // Ejecutar el test
    test_readDirectory();

    // Liberar memoria del buffer
    free(FILE_INFO_BUFFER);

    return 0;
}

//gcc -o test_readDirectory test_readDirectory.c functions.c fileinfo.c -lpthread