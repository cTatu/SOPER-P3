#include "memoriaCompartida.h"

#define OK 0
#define ERROR -1

#define KEY 1300

/**
 * @brief Crea un segmento de memoria compartida
 *
 * Funcion que crea una zona de memoria compartida con el tamanio
 * especificado en size.
 * 
 * @param path Nombre de un fichero del sistema para la generacion de claves
 * @param id_zone El ID de la zona de memoria compartida que se va a crear
 * @param size Cantidad de bytes que se va a reservar.
 * @return Puntero a la zona de memoria o NULL si ya exite la zona o si ha tenido algun otro error.
 */
void* Crear_Memoria_Compartida(void* path, int* id_zone, int size){					

    void* buffer;
    int key = ftok(path, KEY);
    
    if (key == ERROR) {
        perror ("Error with key\n");
        return NULL;
    }

    *id_zone = shmget (key, size, IPC_CREAT | IPC_EXCL |SHM_R | SHM_W);
    
    if (*id_zone == ERROR) {
        perror("Error with id_zone \n");
        return NULL;
    }
    
    buffer = shmat (*id_zone, (void*) 0, 0);
    
    if (buffer == NULL)
        perror("Error reserve shared memory \n");	

    return buffer;
}

/**
 * @brief Borra un segmento de memoria compartida
 *
 * Borra el segmento de memoria identificado a traves del ID
 * desacoplando antes el puntero.
 * 
 * @param buf Puntero de la memoria compartida que se va a desacoplar
 * @param id_zone El ID de la zona de memoria compartida que se va a borrar
 * @return OK o ERROR
 */
int Borrar_Memoria_Compartida(void* buf, int id_zone){
	if (shmdt ((void *)buf) == ERROR){
        perror("Error with memory detach\n");
        return ERROR;
    }

 	if (shmctl (id_zone, IPC_RMID, (struct shmid_ds *)NULL) == ERROR){
        perror("Error with memory remove\n");
        return ERROR;
    }

    return OK;
}