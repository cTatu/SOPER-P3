#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>
#include <errno.h>
#include <sys/shm.h> 
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdbool.h>
#include <signal.h>
#include "semaforos.h"

#define SEMKEY 7454

#define FILEKEY "/bin/cat"
#define KEY 1401

#define NUM_PROC 2	/*!< Numero de procesos a instanciar	*/	

int id_zone;
int sem_id;
	
typedef struct{
     char nombre[80];   /*!< Cadena de 80 caracteres	*/		
     int id;		/*!< Numero entero	*/		
 } Info;
 
 
void manejadorSIGUSR1(int s){
    Info* info =  shmat(id_zone, (char*)0, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
    if(info == NULL){
     fprintf(stderr, "Error reserve shared memory \n");
     return;
    }
             
    printf("nombre %s, id %d \n", info->nombre, info->id);
}
 
/**
* @brief Inicio del programa
*/
int main(void) {
    
    int pid,i,status=-1;
    Info* info;
    int key;
    unsigned short array[1] = {1};
    char cadena[100];
      
    
    if(signal(SIGUSR1, manejadorSIGUSR1) == SIG_ERR){
        printf("No he podido armar la señal");
        exit(EXIT_FAILURE);
    }
    
    Crear_Semaforo(SEMKEY, 1, &sem_id);
        
    key = ftok(FILEKEY, KEY);
    
    if(key == -1){
        fprintf(stderr, "Error with key \n");
        return -1;
    }
    
    id_zone = shmget(key, sizeof(Info), IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);    printf("ID zone shared memory: %i\n", id_zone);
    
    if(id_zone == -1){
         fprintf(stderr, "Error with id_zone \n");
         return -1;;
    }
    
	info =  shmat(id_zone, (char*)0, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
    
    if(info == NULL){
        fprintf(stderr, "Error reserve shared memory \n");
        exit(EXIT_FAILURE);
    }
    
	info->id = 0;
    Crear_Semaforo(SEMKEY, 1, &sem_id);
    printf("ID zone semaforo: %i\n", sem_id);
    Inicializar_Semaforo(sem_id, array);   
    for(i=0; i<NUM_PROC; i++){
            if((pid=fork())<0) {
                printf("Error haciendo fork\n");
                exit(EXIT_FAILURE);
            } else if(pid==0) {
                sleep(rand() % 10);
                
                info =  shmat(id_zone, (char*)0, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
                if(info == NULL){
                    fprintf(stderr, "Error reserve shared memory \n");
                    exit(EXIT_FAILURE);
                }
                
                Down_Semaforo(sem_id, 0, SEM_UNDO);
                
                printf("Introduce una cadena: \n");
		        fflush(stdin);
                fgets(cadena, 255, stdin);
		        cadena[strlen(cadena)-1] = 0;
		        strcpy(info->nombre, cadena);
                info->id++;
                
                kill(getppid(), SIGUSR1);
		        Up_Semaforo(sem_id, 0, SEM_UNDO);
                exit(EXIT_SUCCESS);
                
            } else {
                
            }
    }
    
    while(wait(&status)>0);
    shmdt((char*) info);
    shmctl(id_zone, IPC_RMID, (struct shmid_ds *)NULL);
    Borrar_Semaforo(sem_id);
    exit(EXIT_SUCCESS);
}