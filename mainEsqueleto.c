//IMPORTS

// VARIABLES GLOBALES

//NUM_THREADS = x;
/* 
Struct: {
    directPath
    copyPath
}file_into_t;
*/

//VARIABLES COMPARTIDAS
/*
file_into_t *file_list;
files(int)
current_file(int)
mutex
*/

//Funciones
/*
leer_directorio(direct_src, direct_copy)
abrir el directorio origen recursivamente e insertar los archivos en la lista

un ciclo que recorra el directorio hasta que no haya mas archivos
sumar a la variable files
*/

/*
funcion copy
copiar los archivos al directorio destino

*/

/*
crear bitacora
crear un archivo de csv con la información de los archivos copiados
*/

// MAIN
/*
-> llamar funcion para leer el directorio origen

-> crear los hilos y llamar a la función copy
 pthread_create(x, NULL, funcionCopy, x)
*/
