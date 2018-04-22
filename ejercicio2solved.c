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
#include "memoriaCompartida.h"

#define SEMKEY 7459		/*!< Clave de semaforo	*/

#define OK 0			/*!< Macro OK	*/
#define ERROR -1		/*!< Macro ERROR	*/

#define FILEKEY "/bin/cat"	/*!< Fihcero cualquiera del sistema	*/


/**
* @brief Ejercicio 2 solucionado
*
* Programa que genera N procesos hijos y cuyo padre reservara memoria,
* para una estrcutura con un nombre y un identificador, que sera compartida
* con sus hijos para que cada uno solicite el nombre por pantalla y lo imprima
* junto con el identificador aumentado en uno.
* 
* Este programa ademas incorpora el uso de semaforos para que no se pueda dar que
* todos los hijos pidan el nombre por pantalla o modifiquen los datos de la estructura
* con el nombre y el identificador a la vez.
*  
*
* @file ejercicio2solved.c
* @author Pareja 3 - Grupo 2214
* @author Cristian Tatu cristian.tatu@estudiante.uam.es
* @author Sara Sanz Lucio sara.sanzlucio@estudiante.uam.es
* @date 22-04-2018
*/

/**
* @brief Estructura con una cadena de caracteres y un numero entero
*/
typedef struct{
     char nombre[80];   /*!< Cadena de 80 caracteres	*/		
     int id;		/*!< Numero entero	*/		
 } Info;


/**
 * @brief variables globales
 */
int id_zone;
int sem_id; 	/*!< ID del semaforo	*/
Info* info;		/*!< La variable de estructura a escribir en la memoria	*/

/**
 * @brief manejador de la señal enviada SIGUSR1
 *
 * el manejador hace un attach de la memoria compartida creada en el main
 * e imprime los valores del nombre e identificador existentes en el bloque 
 * memoria compartida
 *
 * @param s la señal
 */
void manejadorSIGUSR1(int s){
    info =  shmat(id_zone, (char*)0, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
    if(info == NULL){
     fprintf(stderr, "Error reserve shared memory \n");
     return;
    }
             
    printf("nombre %s, id %d \n", info->nombre, info->id);
}
 
/**
* @brief Inicio del programa
*/
int main(int argc, char *argv[]) {
    
    int pid,i;
    Info* info;
    unsigned short array[1] = {1};
    char cadena[100];
     int num_proc;
     
     srand(time(NULL));
      
    Crear_Semaforo(rand(), 1, &sem_id);
    info = Crear_Memoria_Compartida(FILEKEY, &id_zone, sizeof(Info));     

    if(signal(SIGUSR1, manejadorSIGUSR1) == SIG_ERR){
        printf("No he podido armar la señal");
        exit(EXIT_FAILURE);
    }

	info->id = 0;
    Crear_Semaforo(SEMKEY, 1, &sem_id);
    Inicializar_Semaforo(sem_id, array);   
    
    num_proc = atoi(argv[1]);
    
    for(i=0; i < num_proc; i++){
            if((pid=fork())<0) {
                printf("Error haciendo fork\n");
                exit(EXIT_FAILURE);
            } else if(pid==0) {
                sleep(rand() % 10);
                
                Down_Semaforo(sem_id, 0, SEM_UNDO);
                
                printf("Introduce una cadena: \n");

                fgets(cadena, 255, stdin);
		        cadena[strlen(cadena)-1] = 0;
		        strcpy(info->nombre, cadena);
                info->id++;
                
                kill(getppid(), SIGUSR1);
		        Up_Semaforo(sem_id, 0, SEM_UNDO);
                exit(EXIT_SUCCESS);
            }
    }
    
    while(wait(NULL)>0);

    if(Borrar_Memoria_Compartida(info, id_zone) == ERROR)
		printf("Error eliminando la memoria compartida");

    Borrar_Semaforo(sem_id);
    exit(EXIT_SUCCESS);
}