#include "semaforos.h"
#include "memoriaCompartida.h"

#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>

#define OK 0
#define ERROR -1

#define FILEKEY "/bin/cat"

typedef enum {productor, consumidor} tipoProceso;

int mutex, sePuedeLeer, sePuedeEscribir;
char* buffer;
int id_zone;

void manejadorControlC(){
	/* Solo le asignamos el controlador al padre 
	*  Los hijos al recibir la senial terminaran
	* y el padre podra limpiar los semaforos y la memoria */

    if (Borrar_Memoria_Compartida( (char*) buffer, id_zone) == ERROR)
        exit(EXIT_FAILURE);

	printf("mutex: %d\n", mutex);
    if (Borrar_Semaforo(mutex) == ERROR){
    	perror("Error borrando el semaforo mutex");
        exit(EXIT_FAILURE);
    }

    printf("sePuedeLeer: %d\n", sePuedeLeer);
    if (Borrar_Semaforo(sePuedeLeer) == ERROR){
    	perror("Error borrando el semaforo sePuedeLeer");
        exit(EXIT_FAILURE);
    }

    printf("sePuedeEscribir: %d\n", sePuedeEscribir);
    if (Borrar_Semaforo(sePuedeEscribir) == ERROR){
    	perror("Error borrando el semaforo sePuedeEscribir");
        exit(EXIT_FAILURE);
    }

}


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
        exit(EXIT_FAILURE);
    }


    int* active = (int*) malloc(sizeof(int) * tamanioBuffer);
    if (active == NULL){
    	printf("Error al reservar memoria\n");
    	exit(EXIT_FAILURE);
    }

    for(i = 0; i < tamanioBuffer; i++)
    	active[i] = 1;

    /* Solo incializamos el mutex a 1 para garantizar la exclusion mutua en cada
    * posicion del array. El semaforo para que se pueda leer lo dejamos en 0 porque
    * inicilamente el array esta vacio                                         */
    if(UpMultiple_Semaforo(mutex, tamanioBuffer, SEM_UNDO, active) == ERROR){
    	perror("Error inicializando el semaforo mutex a 1");
        exit(EXIT_FAILURE);
    }
    
    if (Crear_Semaforo(rand(), tamanioBuffer, &sePuedeLeer) == ERROR){
        perror("Error creando el semaforo sePuedeLeer");
        exit(EXIT_FAILURE);
    }

    if (Crear_Semaforo(rand(), tamanioBuffer, &sePuedeEscribir) == ERROR){
        perror("Error creando el semaforo sePuedeEscribir");
        exit(EXIT_FAILURE);
    }

    if(UpMultiple_Semaforo(sePuedeEscribir, tamanioBuffer, SEM_UNDO, active) == ERROR){
    	perror("Error inicializando el semaforo sePuedeEscribir a 1");
        exit(EXIT_FAILURE);
    }

    producto = '#';

    for(i = 0; i < tamanioBuffer; i++)
    	active[i] = 555;
    
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
			nanosleep((const struct timespec[]){{0, 30000000L}}, NULL);
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
			nanosleep((const struct timespec[]){{0, 30000000L}}, NULL);

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
     	/*Espera Control-C*/
		pause();
		/*Espera a todos los hijos*/
		while(wait(NULL) > 0);

		free(active);
	}

    exit(EXIT_SUCCESS);
    
}
    