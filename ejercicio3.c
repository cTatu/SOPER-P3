#include "semaforos.h"
#include "memoriaCompartida.h"

#define OK 0
#define ERROR -1

#define FILEKEY "/bin/cat"

void manejadorControlC(){
    
}


int main (int argc, char *argv[]) {
    
    int mutex, sePuedeLeer;
    
    int id_zone, tamanioBuffer;
    char* buffer;

    srand(time(NULL));

    if (argc == 1){
        printf("Introduce la longitud del buffer\n\t ej: ./ejercicio3 10\n");
        exit(EXIT_FAILURE);
    }
    
    tamanioBuffer = atoi(argv[1]);
    if (tamanioBuffer < 1){
        printf("La longitud debe ser mayor que 0\n");
        exit(EXIT_FAILURE);
    }
    
    buffer = (char*) Crear_Memoria_Compartida(FILEKEY, &id_zone, tamanioBuffer);
    if (buffer == NULL){ 
        perror("Error reservando memoria compartida");
        exit(EXIT_FAILURE);
    }
    
    if (Crear_Semaforo(rand(), tamanioBuffer, &mutex) == ERROR){
        perror("Error creando el semaforo mutex");
        exit(EXIT_FAILURE);
    }
    
    if (Crear_Semaforo(rand(), tamanioBuffer, &sePuedeLeer) == ERROR){
        perror("Error creando el semaforo sePuedeLeer");
        exit(EXIT_FAILURE);
    }
    

    if (Borrar_Memoria_Compartida( (char*) buffer, id_zone) == ERROR){
    	perror("Error borrando la zona de memoria compartida");
        exit(EXIT_FAILURE);
    }

    if (Borrar_Semaforo(mutex) == ERROR){
    	perror("Error creando el semaforo mutex");
        exit(EXIT_FAILURE);
    }

    if (Borrar_Semaforo(sePuedeLeer) == ERROR){
    	perror("Error creando el semaforo sePuedeLeer");
        exit(EXIT_FAILURE);
    }

    return 1;
    
}
    