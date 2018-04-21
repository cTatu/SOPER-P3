#include "semaforos.h"
#include "memoriaCompartida.h"

#define OK 0
#define ERROR -1

#define FILEKEY "/bin/cat"

typedef enum {padre, productor, consumidor} tipoProceso;

void manejadorControlC(){
	/* Solo le asignamos el controlador al padre 
	*  Los hijos al recibir la senial terminaran
	* y el padre podra limpiar los semaforos y la memoria */

    if (Borrar_Memoria_Compartida( (char*) buffer, id_zone) == ERROR)
        exit(EXIT_FAILURE);

    if (Borrar_Semaforo(mutex) == ERROR){
    	perror("Error borrando el semaforo mutex");
        exit(EXIT_FAILURE);
    }

    if (Borrar_Semaforo(sePuedeLeer) == ERROR){
    	perror("Error borrando el semaforo sePuedeLeer");
        exit(EXIT_FAILURE);
    }

}


int main (int argc, char *argv[]) {
    
    int mutex, sePuedeLeer;
    
    tipoProceso tipo;

    int pid, i, id_zone, tamanioBuffer;
    char* buffer;
    char producto;

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


    int* active = (int*) malloc(sizeof(int) * tamanioBuffer);
    if (active == NULL){
    	printf("Error al reservar memoria\n");
    	exit(EXIT_FAILURE);
    }

    for(i = 0; i < tamanioBuffer; i++)
    	active[i] = 1;

    if(UpMultiple_Semaforo(mutex, tamanioBuffer, SEM_UNDO, active) == ERROR){
    	perror("Error inicializando el semaforo mutex a 1");
        exit(EXIT_FAILURE);
    }
    
    if (Crear_Semaforo(rand(), tamanioBuffer, &sePuedeLeer) == ERROR){
        perror("Error creando el semaforo sePuedeLeer");
        exit(EXIT_FAILURE);
    }
/*
    if(UpMultiple_Semaforo(sePuedeLeer, tamanioBuffer, SEM_UNDO, active) == ERROR){
    	perror("Error inicializando el semaforo sePuedeLeer a 1");
        exit(EXIT_FAILURE);
    }
*/
    tipo = padre;
    
    for(i = 0; i < 2; i++){
	    if ( (pid = fork()) == 0){
	    	if (i == 0)
	    		tipo = productor;
	    	else /* (i == 1) */
	    		tipo = consumidor; 
	    	break;
	    }
	}

	if (tipo == productor){
		producto = 'a';
		while(true){
			if (producto == 'z')
				producto = '0'
			if (producto == '9')
				producto = 'a'

			Down_Semaforo(mutex, );
		}
	}else if (tipo == consumidor){
		while(true){
			
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
	}

    exit(EXIT_SUCCESS);
    
}
    