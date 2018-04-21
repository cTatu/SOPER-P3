#include "semaforos.h"
#include "memoriaCompartida.h"

#define OK 0
#define ERROR -1

#define FILEKEY "/bin/cat"

void manejadorControlC(){
    
}


int main (int argc, char *argv[]) {
    
    int mutex, sePuedeLeer;
    
    int id_zone, i, tamanioBuffer;
    char* buffer;

    if (argc == 1){
        printf("Introduce la longitud del buffer\n\t ej: ./ejercicio3 10\n");
        exit(EXIT_FAILURE);
    }
    
    tamanioBuffer = atoi(argv[1]);
    if (tamanioBuffer < 1){
        printf("La longitud debe ser mayor que 0\n");
        exit(EXIT_FAILURE);
    }
    
    Crear_Memoria_Compartida( (void*) buffer, FILEKEY, id_zone, tamanioBuffer);
    if (buffer == NULL){ 
        perror("Error reservando memoria compartida\n");
        exit(EXIT_FAILURE);
    }
    
    if (Crear_Semaforo(IPC_PRIVATE, tamanioBuffer, &mutex) == ERROR){
        perror("Error creando el semaforo mutex\n");
        exit(EXIT_FAILURE);
    }
    
    if (Crear_Semaforo(IPC_PRIVATE, tamanioBuffer, &sePuedeLeer) == ERROR){
        perror("Error creando el semaforo sePuedeLeer\n");
        exit(EXIT_FAILURE);
    }
    

    if (Borrar_Memoria_Compartida( (void*) buffer, id_zone) == ERROR){
    	perror("Error borrando la zona de memoria compartida\n");
        exit(EXIT_FAILURE);
    }

    if (Borrar_Semaforo(mutex) == ERROR){
    	perror("Error creando el semaforo mutex\n");
        exit(EXIT_FAILURE);
    }

    if (Borrar_Semaforo(sePuedeLeer) == ERROR){
    	perror("Error creando el semaforo sePuedeLeer\n");
        exit(EXIT_FAILURE);
    }
    
}
    