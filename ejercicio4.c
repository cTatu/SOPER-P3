#include "semaforos.h"
#include "memoriaCompartida.h"

#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/**
* @brief Ejercicio 4
*
* Programa que invoca a un hilo que escribira en un fichero un numero n de numeros
* aleatorios, tras esto se esperara al hilo y se creara otro hilo que leera este fichero
* usando mmap(). Este mapeo se utilizara para que el hilo, a partir de la direccion 
* de memoria mapeada, cambie los espacios del fichero por comas.
*  
*
* @file ejercicio4.c
* @author Pareja 3 - Grupo 2214
* @author Cristian Tatu cristian.tatu@estudiante.uam.es
* @author Sara Sanz Lucio sara.sanzlucio@estudiante.uam.es
* @date 22-04-2018
*/

#define OK 0		/*!< Macro OK	*/
#define ERROR -1	/*!< Macro ERROR	*/

char* map;			/*!< Puntero a la memoria virtual del fichero	*/

/**
 * @brief numeroAleatorioRango
 *
 * la funcion calcula un numero aleatorio entre los dos numeros que se pasan por 
 * argumento
 * @param extremoInferior numero inferior del rango del que se sacara el numero aleatorio
 * @param extremoSuperior numero superior del rango del que se sacara el numero aleatorio
 * @return numero aleatorio obtenido
 */
unsigned short numeroAleatorioRango(unsigned short extremoInferior, unsigned short extremoSuperior){

	/* Obtenemos en porcentaje aleatorio que lo usamos para
	* darle peso a cada extremo                       */
	double porcentaje = ((double)rand()) / RAND_MAX;

	/* Al extremo superior le damos el peso normal */
	double numSuperior = porcentaje * extremoSuperior;
	/* Al extremo inferior le damos el peso complementario */
	double numInferior = (1-porcentaje) * extremoInferior;

	/*EJ:	porcentaje = 0.5  extremoInferior = 5  extremoSuperior = 10
	*		numInferior = (1-0.5) * 5 = 0.5 * 5 = 2.5        numSuperior = 0.5 * 10 = 5
	*       result = 2.5 + 5 = 7.5                                       
	*
	*       porcentaje = 1.0  extremoInferior = 5  extremoSuperior = 10
	*		numInferior = (1-1.0) * 5 = 0.0 * 5 = 0        numSuperior = 1.0 * 10 = 10
	*       result = 0 + 10 = 10
	*/
	unsigned short numeroAleatorio = (unsigned short) (numInferior + numSuperior);

	return numeroAleatorio + 1;
} 

/**
 * @brief escribeFichero
 *
 * funcion que escribe los numeros aleatorios obtenidos en la funcion numeroAleatorioRango()
 * en el fichero creado 
 */
void* escribeFichero()
{
    FILE* file;
    unsigned short n;

    file = fopen("numerosAleat.txt", "w");

    n = numeroAleatorioRango(1000, 2000);

    fprintf(file, "%u", numeroAleatorioRango(100, 1000));

    while(n > 1){
    	fprintf(file, ",%u", numeroAleatorioRango(100, 1000));
    	n--;
    }

    fclose(file);

    return NULL;
}

/**
 * @brief lee el fichero
 *
 * la funcion el fichero en el que se han escrito anteriormente los numeros aleatorios generados
 * @param descriptor del fichero en el que se ha escrito anteriormente
 */
void* leeFichero(void* descriptor)
{
  	struct stat buffer;
	long int i,tamanio;
    int descrp;
    
    descrp = *((int*)descriptor);

	fstat( descrp, &buffer);

	tamanio = buffer.st_size;

	map = (char*) mmap(0, tamanio, PROT_READ | PROT_WRITE, MAP_SHARED, descrp, 0);
    if (map == MAP_FAILED) {
		close(descrp);
		perror("Error mmapping the file");
		exit(EXIT_FAILURE);
    }

	for(i = 0; i < tamanio; i++){
		if (map[i] == ',')
			map[i] = ' ';
	}

	if (msync(map, tamanio, MS_SYNC) == ERROR)
    {
        perror("Could not sync the file to disk");
    }

	printf("Numeros: %s\n", map);

	if (munmap(map, tamanio) == ERROR)
		perror("Error al liberar el mapeo");

    return NULL;
}
/**
 * @brief Inicio del programa
 */
int main (int argc, char *argv[]) {
    pthread_t escribeAleatID, leeAleatID;
    int descriptor;

    srand(time(NULL));

    if (argc != 2){
        printf("Argumentos invalidos:\n\tEj: ejercicio4 fichero.txt\n");
        exit(EXIT_FAILURE);
    }

    /* Lanza el hilo */
    pthread_create(&escribeAleatID, NULL, escribeFichero, (void*) argv[1]);
    /* Le espera */
    pthread_join(escribeAleatID, NULL);

    descriptor = open(argv[1], O_RDWR);
    if (descriptor == ERROR) {
		perror("Error al abrir el fichero para leer");
		exit(EXIT_FAILURE);
    }    

    pthread_create(&leeAleatID, NULL, leeFichero, (void*) &descriptor);
    /* Le espera */
    pthread_join(leeAleatID, NULL);

    close(descriptor);

    exit(EXIT_SUCCESS);
}
    