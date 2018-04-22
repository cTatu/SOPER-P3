#include "semaforos.h"
#include "memoriaCompartida.h"

#include <sys/wait.h>
#include <sys/msg.h>
#include <ctype.h>
#include <stdbool.h>
#include <signal.h>

#define OK 0		/*!< Macro OK	*/
#define ERROR -1	/*!< Macro ERROR	*/

int colaMensajesAB;	/*!< Cola de mensajes para el proceso A y B	*/
int colaMensajesBC;	/*!< Cola de mensajes para el proceso B y C	*/

/**
* @brief Ejercicio 5 (cadena de montaje)
*
* Programa que lanza 3 procesos hijos donde cada uno hace una tarea distinta.
* El primero lee datos de un fichero en segmentos de 2KB y se los envia a traves
* de mensajes al siguiente proceso. Este se encarga de modificar todos los caracteres
* al siguiente valor alfabetico. Y el ultimo proceso recibe esa cadena modificada
* a traves de otro mensaje y la escribe en otro fichero.
* Los ficheros deben especificarse a traves de los argumentos de entrada del ejecutable.*  
*
* @file cadena_montaje.c
* @author Pareja 3 - Grupo 2214
* @author Cristian Tatu cristian.tatu@estudiante.uam.es
* @author Sara Sanz Lucio sara.sanzlucio@estudiante.uam.es
* @date 22-04-2018
*/

/** Struct de Mensaje 
* La estructura donde se guarda el mensaje
*/
typedef struct _Mensaje{
        long id; 			 /*!< Campo obligatorio a long que identifica el tipo de mensaje	*/	
        char texto[2000];	 /*!< Informacion a transmitir en el mensaje	*/	
    }mensaje;


/**
 * @brief Proceso A - leeFichero
 *
 * Funcion del proceso A. Lee el fichero indicado en segmentos de 2KB y los envia
 * a traves de una cola de mensajes al proceso B para su procesamiento.
 * 
 * @param filename Nombre del fichero que se va a leer
 * @return void
 */
void leeFichero(char* filename){
    mensaje msg;

    msg.id = 1;

    FILE* file;

	/*Abrimos el fichero para leerlo*/
    file = fopen(filename, "r");
    if (file == NULL){
        perror("El fichero no se ha podido abrir para lectura");
        exit(EXIT_FAILURE);
    }

	/*Leemos trozos de 2000 bytes (2KB) y los cargamos directo
	* en la estructura del mensaje
	*/
    while(fgets(msg.texto, 2000, file) != NULL)
    	/* Enviamos la estructura (con el mensaje de 2KB dentro) a la cola de mensajes 
    	* en la cola "colaMensajesAB" 
		*/
        msgsnd (colaMensajesAB, (struct msgbuf *) &msg, sizeof(mensaje) - sizeof(long), IPC_NOWAIT);
    if(!feof(file))
        printf("Error leyendo el archivo\n");

    fclose(file);

	/* Cuando se acaba el bucle se ha leido todo el fichero asi que enviamos
	* un mensaje al otro proceso con '\0' para que sepa que no habra mas
	*/
    *msg.texto = '\0';
    msgsnd (colaMensajesAB, (struct msgbuf *) &msg, sizeof(mensaje) - sizeof(long), IPC_NOWAIT);

    exit(EXIT_SUCCESS);
}

/**
 * @brief Proceso B - modificaMensaje
 *
 * Funcion del proceso B. Recibe el mensaje con el segmento de 2KB de datos
 * y recorre todo el array cambiando cada letra alfabetica por la consecutiva.
 * 
 * @return void
 */
void modificaMensaje(){
    mensaje msg;
    char* puntero;

    while(true){
    	/* Se queda esperando a que haya un mensaje, cuando lo hay lo lee 
    	* de la cola "colaMensajesAB" 
		*/
        msgrcv (colaMensajesAB, (struct msgbuf *) &msg, sizeof(mensaje) - sizeof(long), 1, 0);
        /* Usamos un puntero auxiliar para recorrer el array */
        puntero = msg.texto;

		/* Mientras que no haya llegado al fin del array */
        while(*puntero != '\0'){
        	/* Solo cambiamos los caracteres que son letras del abecedario */
            if (isalpha(*puntero)){
                if (*puntero == 'z' || *puntero == 'Z')
                    *puntero = 'a';
                else
                    (*puntero)++; /* Incrementar el valor del elemento apuntado por el puntero (b -> c) */
            }
            puntero++; /* Mover el puntero a la siguiente posicion */
        }

		/* Despues de acabar de modificar los datos, se los enviamos al siguiente proceso para que lo 
		* escriba en un fichero a traves de la cola "colaMensajesBC"
		*/
        msgsnd (colaMensajesBC, (struct msgbuf *) &msg, sizeof(mensaje) - sizeof(long), IPC_NOWAIT);
        /* Si el mensaje anterior fue un fin '\0' terminamos y tambien se lo hemos enviado al C */
        if (*msg.texto == '\0')
            break;
    }

    exit(EXIT_SUCCESS);
}

/**
 * @brief Proceso C - escribeFichero
 *
 * Funcion del proceso C. Escribe en el fichero los datos recibidos
 * del proceso B que han sido modificados con el desplazamiento alfabetico.
 * 
 * @param filename Nombre del fichero que se va a escribir
 * @return void
 */
void escribeFichero(char* filename){
    mensaje msg;

    FILE* file;

	/* Abrir otro fichero para escribir los datos modificados del primer fichero */
    file = fopen(filename, "w");
    if (file == NULL){
        perror("El fichero no se ha podido abrir para escritura");
        exit(EXIT_FAILURE);
    }

    while(true){
    	/* Recibir los datos modificados del B en la cola "colaMensajesBC" */
        msgrcv (colaMensajesBC, (struct msgbuf *) &msg, sizeof(mensaje) - sizeof(long), 1, 0);

		/* Imprimirlos en el fichero */
        fprintf(file, "%s", msg.texto);

        if (*msg.texto == '\0')
            break;
    }

    fclose(file);

    exit(EXIT_SUCCESS);
}

/**
 * @brief Inicio del programa
 * 
 * Se crean las dos colas de mensajes y se lanzan los 3 procesos
 * asignandoles a cada uno una tarea/funcion determinada.
 * 
 * @param argc Numero de argumentos que recibe el ejecutable
 * @param argv Puntero a todos los argumentos del programa
 * @return EXIT_SUCCESS o EXIT_FAILURE
 */
int main (int argc, char *argv[]) {
    int i, pid;
    key_t clave;

    if (argc != 3){
        printf("Argumentos invalidos:\n\tEj: cadena_montaje random.txt desp.txt\n");
        exit(EXIT_FAILURE);
    }

	/* 2 Colas de mensajes */
    for(i = 0; i < 2; i++){
    	
    	/* Clave para cada cola de mensajes */
        clave = ftok ("/bin/ls", i);
        if (clave == (key_t) -1){
            perror("Error al obtener clave para cola mensajes");
            exit(EXIT_FAILURE);
        }

        if (i == 0){
        	/* Crear la cola entre el proceso A y B */
            colaMensajesAB = msgget (clave, 0600 | IPC_CREAT);
            if (colaMensajesAB == ERROR){
                perror("Error al obtener identificador para colaMensajesAB");
                exit(EXIT_FAILURE);
            }
        }else if (i == 1){
        	/* Crear la cola entre el proceso B y C */
            colaMensajesBC = msgget (clave, 0600 | IPC_CREAT);
            if (colaMensajesBC == ERROR){
                perror("Error al obtener identificador para colaMensajesBC");
                exit(EXIT_FAILURE);
            }
        }
    }
    
	/* 3 procesos, lector, modificador y escritor */
    for(i = 0; i < 3; i++){
        if ( (pid = fork()) == 0){
            switch(i){
                case 0:
                    leeFichero(argv[1]);
                case 1:
                    modificaMensaje();
                case 2:
                    escribeFichero(argv[2]);
            }
        }
    }

    signal(SIGINT, SIG_IGN);  

	/* Esperar a los hijos */
    while(wait(NULL) > 0);

	/* Borrar las colas de mensajes */
    msgctl (colaMensajesAB, IPC_RMID, (struct msqid_ds*) NULL);
    msgctl (colaMensajesBC, IPC_RMID, (struct msqid_ds*) NULL);

    exit(EXIT_SUCCESS);
}
    