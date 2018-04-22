#include "semaforos.h"
#include "memoriaCompartida.h"

#include <pthread.h>

unsigned short numeroAleatorioRango(unsigned short extremoInferior, 
									unsigned short extremoSuperior){

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
	unsigned short numeroAleatorio = ((unsigned short) numInferior) + ((unsigned short) numSuperior);

	return numeroAleatorio + 1;
} 

void* escribeFichero(void *vargp)
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

int main (int argc, char *argv[]) {
    pthread_t tid;

    srand(time(NULL));

    pthread_create(&tid, NULL, escribeFichero, NULL);
    pthread_join(tid, NULL);

    exit(EXIT_SUCCESS);
}
    