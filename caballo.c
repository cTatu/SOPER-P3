#include "memoriaCompartida.h"
#include "caballo.h"

int posicion, IDc;
TipoTirada tirada;
Parametros* params;

int numeroAleatorioRango(int extremoInferior, int extremoSuperior){

	double porcentaje = ((double)rand()) / RAND_MAX;

	double numSuperior = porcentaje * (extremoSuperior + 1);
	double numInferior = (1-porcentaje) * extremoInferior;
    
	int numeroAleatorio = (int) (numInferior + numSuperior);

	return numeroAleatorio;
} 

void liberar(){
    int i, j;
    for(i = 0; i < params->numCaballos; i++){
        for(j = 0; j < 2; j++){
            close(params->pipesCaballos[i][j][READ]);
            close(params->pipesCaballos[i][j][WRITE]);
        }
    }
    
    for(i = 0; i < params->numCaballos; i++){
        for(j = 0; j < 2; j++)
            free(params->pipesCaballos[i][j]);
        free(params->pipesCaballos[i]);
    }
    
    free(params->pipesCaballos);
    free(params->pidsCaballos);
    
    free(params);
}

void manejadorINTNormalCaballo(){
    liberar();
    
    exit(EXIT_SUCCESS);
}

void manejadorINTForzosaCaballo(){
    liberar();
    
    exit(EXIT_FAILURE);
}

void manejadorCalculoTirada(){
    TipoTirada tipoT;
    int tirada = 0;
    
    read(params->pipesCaballos[IDc][PARENT][READ], &tipoT, sizeof(tirada));
    
    if (IDc == 0)
        printf("\n");
    printf("Caballo: %d Pos: %d\n", IDc, posicion);
            
    switch(tipoT){
        case NORMAL:
            default:
                tirada = numeroAleatorioRango(1,6);
                printf("\t NORMAL: %d\n", tirada);
            break;
        case PRIMERO:
                tirada = numeroAleatorioRango(1,7);
                printf("\t PRIMERO: %d\n", tirada);
            break;
        case ULTIMO:
                tirada = numeroAleatorioRango(1,6);
                tirada += numeroAleatorioRango(1,6);
                printf("\t ULTIMO: %d\n", tirada);
            break;        
    }
    posicion += tirada;
    
    //printf("Write IN:%d   %d\n", IDc, tirada);
    write(params->pipesCaballos[IDc][CHILD][WRITE], &tirada, sizeof(tirada));

}

int rutina_caballo(int ID){
    sigset_t mask, oldmask;
    posicion = 0;
    
    srand(time(NULL) + ID);
    IDc = ID;
    
    sigemptyset(&mask);
	
	if (signal(SIGINT, manejadorINTForzosaCaballo) == SIG_ERR){
		perror("Error en la captura de SIGNINT");
		liberar();
        return ERROR;
	}
	if (signal(SIGUSR1, manejadorCalculoTirada) == SIG_ERR){
		perror("Error en la captura de SIGUSR1");
		liberar();
        return ERROR;
	}
    if (signal(SIGUSR2, manejadorINTNormalCaballo) == SIG_ERR){
		perror("Error en la captura de SIGUSR2");
		liberar();
        return ERROR;
	}
    
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
	sigprocmask (SIG_BLOCK, &mask, &oldmask);
	while(true)
        sigsuspend(&oldmask);
    sigprocmask (SIG_UNBLOCK, &mask, NULL);
}