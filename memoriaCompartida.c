#include "memoriaCompartida.h"

#define OK 0
#define ERROR -1

#define KEY 1300

char* Crear_Memoria_Compartida(char* path, int* id_zone, int size){					

    char* buffer;
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
    
    printf ("ID zone shared memory: %i\n", *id_zone);

    buffer = shmat (*id_zone, (char*) 0, 0);
    
    if (buffer == NULL)
        perror("Error reserve shared memory \n");	

    return buffer;
}

int Borrar_Memoria_Compartida(char* buf, int id_zone){
	if (shmdt ((char *)buf) == ERROR){
        perror("Error with memory detach\n");
        return ERROR;
    }

 	if (shmctl (id_zone, IPC_RMID, (struct shmid_ds *)NULL) == ERROR){
        perror("Error with memory remove\n");
        return ERROR;
    }

    return OK;
}