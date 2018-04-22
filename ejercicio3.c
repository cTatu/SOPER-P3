#include "semaforos.h"
#include "memoriaCompartida.h"

#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>

#define OK 0 		/*!< Macro OK	*/
#define ERROR -1	/*!< Macro ERROR	*/

#define FILEKEY "/bin/cat"		/*!< Fichero cualquiera del sistema	*/

/**
* @brief Ejercicio 3
*
* Programa que consta de un padre que genera dos procesos hijos, uno de ellos 
* sera el productor y el otro el consumidor. Se creara un bloque de memoria 
* compartida para que el productor guarde las letras y numeros que produzca y
* el consumidor las saque. Se usaran semaforos dar permiso al productor y al consumidor
* de escribir en el buffer o leer del buffer respectivamente y que no se produzcan 
* problemas como que no existan caracteres que leer en el buffer.
*
* @file ejercicio3.c
* @author Pareja 3 - Grupo 2214
* @author Cristian Tatu cristian.tatu@estudiante.uam.es
* @author Sara Sanz Lucio sara.sanzlucio@estudiante.uam.es
* @date 22-04-2018
*/

/**
 * @brief enumerado con el proceso a ejecutar, sea el productor o el consumidor
 */
typedef enum {productor, consumidor} tipoProceso;

int mutex;			/*!< ID de semaforo mutex	*/
int sePuedeLeer;	/*!< ID de semaforo se puede leer	*/
int sePuedeEscribir;/*!< ID de semaforo se puede escribir	*/
char* buffer;		/*!< puntero a la zona de memoria	*/
int id_zone;		/*!< ID de la zona de memoria	*/

/**
 * @brief manejador de la señal Control C
 *
 * El manejador se encargara de capturar la señal de Control C, siendo 
 * la unica forma de que este finalice ya que la produccion y el consumo de caracteres es infinito 
 * liberando todos los recursos utilizado, como los semaforos y la memoria compartida.
 */
void manejadorControlC(){
	/* Solo le asignamos el controlador al padre 
	*  Los hijos al recibir la senial terminaran
	* y el padre podra limpiar los semaforos y la memoria */

    if (Borrar_Memoria_Compartida( (void*) buffer, id_zone) == ERROR)
        exit(EXIT_FAILURE);

    if (Borrar_Semaforo(mutex) == ERROR){
    	perror("Error borrando el semaforo mutex");
        exit(EXIT_FAILURE);
    }

    if (Borrar_Semaforo(sePuedeLeer) == ERROR){
    	perror("Error borrando el semaforo sePuedeLeer");
        exit(EXIT_FAILURE);
    }

    if (Borrar_Semaforo(sePuedeEscribir) == ERROR){
    	perror("Error borrando el semaforo sePuedeEscribir");
        exit(EXIT_FAILURE);
    }

}

/**
 * @brief Inicio del programa
 */
int main (int argc, char *argv[]) {
    
    tipoProceso tipo;
    int pid, i, tamanioBuffer;
    
    char producto;

    srand(time(NULL));

    if (argc != 3){
        printf("Introduce la longitud del buffer y que proceso quieres que empiece primero\n\t ej: ./ejercicio3 10 p\n\t ej: ./ejercicio3 10 c\n");
        exit(EXIT_FAILURE);
    }if (*argv[2] != 'p' && *argv[2] != 'c'){
    	printf("La inicial del proceso debe ser:\n\t c (consumidor)\n\t p (productor)\n");
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
        Borrar_Memoria_Compartida( (void*) buffer, id_zone);
        exit(EXIT_FAILURE);
    }


    int* active = (int*) malloc(sizeof(int) * tamanioBuffer);
    if (active == NULL){
    	printf("Error al reservar memoria\n");
    	Borrar_Memoria_Compartida( (void*) buffer, id_zone);
    	Borrar_Semaforo(mutex);
    	exit(EXIT_FAILURE);
    }

    for(i = 0; i < tamanioBuffer; i++)
    	active[i] = 1;

    /* Solo incializamos el mutex a 1 para garantizar la exclusion mutua en cada
    * posicion del array. El semaforo para que se pueda leer lo dejamos en 0 porque
    * inicilamente el array esta vacio                                         */
    if(UpMultiple_Semaforo(mutex, tamanioBuffer, SEM_UNDO, active) == ERROR){
    	perror("Error inicializando el semaforo mutex a 1");
    	Borrar_Memoria_Compartida( (void*) buffer, id_zone);
    	Borrar_Semaforo(mutex);
        exit(EXIT_FAILURE);
    }
    
    if (Crear_Semaforo(rand(), tamanioBuffer, &sePuedeLeer) == ERROR){
        perror("Error creando el semaforo sePuedeLeer");
        Borrar_Memoria_Compartida( (void*) buffer, id_zone);
    	Borrar_Semaforo(mutex);
        exit(EXIT_FAILURE);
    }

    if (Crear_Semaforo(rand(), tamanioBuffer, &sePuedeEscribir) == ERROR){
        perror("Error creando el semaforo sePuedeEscribir");
        Borrar_Memoria_Compartida( (void*) buffer, id_zone);
    	Borrar_Semaforo(mutex);
    	Borrar_Semaforo(sePuedeLeer);
        exit(EXIT_FAILURE);
    }

    if(UpMultiple_Semaforo(sePuedeEscribir, tamanioBuffer, SEM_UNDO, active) == ERROR){
    	perror("Error inicializando el semaforo sePuedeEscribir a 1");
    	Borrar_Memoria_Compartida( (void*) buffer, id_zone);
    	Borrar_Semaforo(mutex);
    	Borrar_Semaforo(sePuedeEscribir);
    	Borrar_Semaforo(sePuedeLeer);
        exit(EXIT_FAILURE);
    }

    producto = '#';
    
    for(i = 0; i < 2; i++){
	    if ( (pid = fork()) == 0){
	    	if (i == 0)
	    		tipo = productor;
	    	else /* (i == 1) */
	    		tipo = consumidor; 
	    	break;
	    }
	}

	if (tipo == consumidor){
		if (*argv[2] == 'p')
			nanosleep((const struct timespec[]){{0, 300L}}, NULL);
		i = 0;
		while(true){
			Down_Semaforo(sePuedeLeer, i, SEM_UNDO);
			Down_Semaforo(mutex, i, SEM_UNDO);
				printf("Consumidor lee \"%c\" en posición %d\n", buffer[i], i);
			Up_Semaforo(mutex, i, SEM_UNDO);
			Up_Semaforo(sePuedeEscribir, i, SEM_UNDO);
			i++;
			i = i % tamanioBuffer;
		}
	}else if (tipo == productor){
		if (*argv[2] == 'c')
			nanosleep((const struct timespec[]){{0, 300L}}, NULL);

		producto = 'a';
		i = 0;
		while(true){
			if (producto == 'z' + 1)
				producto = '0';
			if (producto == '9' + 1)
				producto = 'a';
			
			Down_Semaforo(sePuedeEscribir, i, SEM_UNDO);
			Down_Semaforo(mutex, i, SEM_UNDO);
				buffer[i] = producto;
				printf("Productor escribe \"%c\" en posición %d\n", producto, i);
			Up_Semaforo(mutex, i, SEM_UNDO);
			Up_Semaforo(sePuedeLeer, i, SEM_UNDO);
			i++;
			producto++;
			i = i % tamanioBuffer;
		}
	}else /* PADRE*/ {
		if(signal(SIGINT,manejadorControlC) == SIG_ERR){
	    	printf("Error de captura de senial ControlC");
	        manejadorControlC();
	        exit(EXIT_FAILURE);
     	}
		/*Espera a todos los hijos*/
		while(wait(NULL) > 0);
		free(active);
	}

    exit(EXIT_SUCCESS);
    
}
    