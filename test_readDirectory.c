#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include "fileinfo.h"


// Mock para FILE_INFO_BUFFER
FileInfoBuffer *FILE_INFO_BUFFER;

void writeFileInfo(FileInfoBuffer *buffer, FileInfo *fileInfo) {
    // Aquí simplemente imprimimos la información del archivo para verificar manualmente los resultados
    printf("Archivo origen: %s, Archivo destino: %s, Tamaño: %ld bytes\n", fileInfo->origin, fileInfo->destination, fileInfo->size);
}

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

    // Llamar a la función a probar
    readDirectory(sourceDir, destDir);

    // Limpiar archivos y directorios de prueba
    unlink(filePath1);  // Eliminar archivos
    unlink(filePath2);
    rmdir(sourceDir);    // Eliminar directorios
    rmdir(destDir);
}

int main() {
    // Inicializar buffers
    FILE_INFO_BUFFER = malloc(sizeof(FileInfoBuffer));

    // Ejecutar el test
    test_readDirectory();

    // Liberar memoria del buffer
    free(FILE_INFO_BUFFER);

    return 0;
}
