#include "caballo.h"
#include "memoriaCompartida.h"
#include "semaforos.h"

#include <sys/wait.h>
#include <limits.h>

#define FINISH 1

#define FILEKEY "/bin/cat"

Parametros* params;
int* memCompartida;
int* posicionCaballosMC;
int* posiciones_caballos = NULL;
int* nuevas_posiciones = NULL;
int id_zone, mutex_MC;

#define INICIO memCompartida[0]
#define META memCompartida[1]

void nextTiradas(int* tiradas){
    int max = 0;
    int min = INT_MAX;
    int i;
    
    for(i = 0; i < params->numCaballos; i++){
        if (posicionCaballosMC[i] > max)
            max = posicionCaballosMC[i];
        if (posicionCaballosMC[i] < min)
            min = posicionCaballosMC[i];
    }
    
    for(i = 0; i < params->numCaballos; i++){
        if (posicionCaballosMC[i] == max)
            tiradas[i] = PRIMERO;
        else if (posicionCaballosMC[i] == min)
            tiradas[i] = ULTIMO;
        else
            tiradas[i] = NORMAL;
    }
}

void manejadorControlC(){
    int i, j;
    
    for(i = 0; i < params->numCaballos; i++){
        for(j = 0; j < 2; j++){
            close(params->pipesCaballos[i][j][READ]);
            close(params->pipesCaballos[i][j][WRITE]);
        }
    }
    if (posiciones_caballos != NULL)
        free(posiciones_caballos);
    if (nuevas_posiciones != NULL)
        free(nuevas_posiciones);
    
    for(i = 0; i < params->numCaballos; i++)
        wait(&params->pidsCaballos[i]);
    
    if (Borrar_Memoria_Compartida( (void*) memCompartida, id_zone) == ERROR)
        exit(EXIT_FAILURE);
    
    for(i = 0; i < params->numCaballos; i++){
        free(params->pipesCaballos[i]);
    }
    free(params->pipesCaballos);
    
    free(params->pidsCaballos);    
    free(params);
    
    if (Borrar_Semaforo(mutex_MC) == ERROR){
    	perror("Error borrando el semaforo mutex_MC");
        exit(EXIT_FAILURE);
    }
    
    exit(EXIT_SUCCESS);
}

int rutina_etapa_carrera(Parametros* params){
	int i, tirada_calculo;
    int* tiradas;
    
    tiradas = (int*) malloc(sizeof(int) * params->numCaballos);
    if (tiradas == NULL){
        printf("La reserva de memoria para las tiradas de los caballos ha fallado\n");
        exit(EXIT_FAILURE);
    }
    
	posiciones_caballos = (int*) malloc(sizeof(int) * params->numCaballos);
    if (posiciones_caballos == NULL){
        printf("La reserva de memoria para las posiciones de los caballos ha fallado\n");
        exit(EXIT_FAILURE);
    }
    
    nuevas_posiciones = (int*) malloc(sizeof(int) * params->numCaballos);
    if (nuevas_posiciones == NULL){
        printf("La reserva de memoria para las posiciones nuevas de los caballos ha fallado\n");
        exit(EXIT_FAILURE); 
    }
    /*1 - Leo de MEMORIA COMPARTIDA (MC) las posiciones de los caballos (SEMÁFOROS) y las guardo en
    posiciones caballos. Si las posiciones están a 0 es que es la primera ejecución, inicio = True.*/
    Down_Semaforo(mutex_MC, 0, SEM_UNDO);
        memcpy(posiciones_caballos, posicionCaballosMC, sizeof(int) * params->numCaballos);
    Up_Semaforo(mutex_MC, 0, SEM_UNDO);
    
	// Nota: ¿y si el padre recibe Ctrl-C aquí?

    if (!META){
        if (INICIO){
            memset(tiradas, NORMAL, sizeof(int) * params->numCaballos);
            INICIO = false;
        }else
            nextTiradas(tiradas);
        for(i = 0; i < params->numCaballos; i++){
            write(params->pipesCaballos[i][PARENT][WRITE], &tiradas[i], sizeof(int));  
            
            kill(params->pidsCaballos[i], SIGUSR1);
            
            read(params->pipesCaballos[i][CHILD][READ], &tirada_calculo, sizeof(tirada_calculo));
            
            nuevas_posiciones[i] = posiciones_caballos[i] + tirada_calculo;

            if (META == false && nuevas_posiciones[i] >= params->longitudCarrera)
                META = true;
		}
        Down_Semaforo(mutex_MC, 0, SEM_UNDO);
            memcpy(posicionCaballosMC,nuevas_posiciones, sizeof(int) * params->numCaballos);
        Up_Semaforo(mutex_MC, 0, SEM_UNDO);
    }else{
        printf("FINISH\n");
        for(i = 0; i < params->numCaballos; i++)
            kill(params->pidsCaballos[i], SIGUSR2);
        free(tiradas);
        free(posiciones_caballos);
        free(nuevas_posiciones);
        return FINISH;
    }

    free(tiradas);
	free(posiciones_caballos);
    free(nuevas_posiciones);

	return OK;
}


/* Parametros: Num Caballos MAX 10
               Long Carrera
               Num Apostadores MAX 100
               Num Ventanillas
               Cantidad Dinero Disp
*/
int main(int argc, char** argv) {
    int i,j;
    
    if (argc != 6){
        printf("Parametros erroneos!\n\t Uso:\n\t carrera 10 500 100 50 200\n");
        exit(EXIT_FAILURE); 
    }
    
    params = (Parametros*) malloc(sizeof(Parametros));
    if (params == NULL){
        printf("La reserva de memoria para la estructura de parametros ha fallado\n");
        exit(EXIT_FAILURE); 
    }
    
    params->numCaballos = strtoul(argv[1], NULL, 10);
    if (params->numCaballos > 10){
        printf("El numero maximo de caballos es 10\n");
        exit(EXIT_FAILURE); 
    }if (params->numCaballos < 2){
        printf("El numero de caballos no es valido\n");
        exit(EXIT_FAILURE); 
    }
    
    params->longitudCarrera = strtoul(argv[2], NULL, 10);
    if (params->longitudCarrera < 1){
        printf("La longitud de la carrera no es valida\n");
        exit(EXIT_FAILURE); 
    }
    
    params->numApostadores = strtoul(argv[3], NULL, 10);
    if (params->numApostadores > 100){
        printf("El numero maximo de apostadores es 100\n");
        exit(EXIT_FAILURE); 
    }if (params->numApostadores < 1){
        printf("El numero de apostadores no es valido\n");
        exit(EXIT_FAILURE); 
    }
    
    params->numVentanillas = strtoul(argv[4], NULL, 10);
    if (params->numVentanillas < 1){
        printf("El numero de ventanillas no es valido\n");
        exit(EXIT_FAILURE); 
    }
    
    params->cantidadDinero = strtoul(argv[5], NULL, 10);
    if (params->cantidadDinero < 1){
        printf("La cantidad de dinero de los apostadores no es valida\n");
        exit(EXIT_FAILURE); 
    }
    
    params->pidsCaballos = (int*) malloc(sizeof(int) * params->numCaballos);
    if (params->pidsCaballos == NULL){
        printf("La reserva de memoria para los caballos ha fallado\n");
        exit(EXIT_FAILURE); 
    }
    
    params->pipesCaballos = (int***) malloc(sizeof(int**) * params->numCaballos);
    if (params->pipesCaballos == NULL){
        printf("La reserva de memoria para los pipes de caballos ha fallado\n");
        free(params->pidsCaballos);
        exit(EXIT_FAILURE); 
    }
    
    for(i = 0; i < params->numCaballos; i++){
        params->pipesCaballos[i] = (int**) malloc(sizeof(int*) * 2);
        if (params->pipesCaballos[i] == NULL){
            printf("La reserva de memoria para los pipes de hijo y padre ha fallado\n");
            free(params->pidsCaballos);
            free(params->pipesCaballos);
            exit(EXIT_FAILURE); 
        }
        
        for(j = 0; j < 2; j++){
            params->pipesCaballos[i][j] = (int*) malloc(sizeof(int) * 2);
            if (params->pipesCaballos[i][j] == NULL){
                printf("La reserva de memoria para el pipe %d:%d ha fallado\n", i,j);
                free(params->pidsCaballos);
                free(params->pipesCaballos);
                exit(EXIT_FAILURE); 
            }
        }
    }    
    
    
    for(i = 0; i < params->numCaballos; i++){
        for(j = 0; j < 2; j++){
             if (pipe (params->pipesCaballos[i][j]) == ERROR){
              printf("La creacion del pipe %d:%d ha fallado.\n", i,j);
              exit(EXIT_FAILURE); 
            }
        }            
    }
   

    for(i = 0; i < params->numCaballos; i++){
        params->pidsCaballos[i] = fork();
        if (params->pidsCaballos[i] == 0){
            if(rutina_caballo(i) == OK)
                exit(EXIT_SUCCESS);
            else
                exit(EXIT_FAILURE);
        }
    }
    
    memCompartida = (int*) Crear_Memoria_Compartida(FILEKEY, &id_zone, params->numCaballos + 2);
    memset(memCompartida, 0, sizeof(int) * params->numCaballos + 2);
    posicionCaballosMC = &memCompartida[2];
    INICIO = true;
    META = false;
    
    if (Crear_Semaforo(rand(), 1, &mutex_MC) == ERROR){
        perror("Error creando el semaforo mutex_MC");
        liberar();
        exit(EXIT_FAILURE);
    }
    
    if (Up_Semaforo(mutex_MC, 0, SEM_UNDO) == ERROR){
    	perror("Error inicializando el semaforo mutex_MC a 1");
    	liberar();
        exit(EXIT_FAILURE);
    }

    
    signal(SIGINT, manejadorControlC);
    
    // kill(getpid(),SIGINT);
    
    while(rutina_etapa_carrera(params) != FINISH);
    
    /*
    # Comienza la carrera
    # INICIALIZACIÓN VARIABLES EN MEMORIA COMPARTIDA ¿Estructura?
    # Lanzamiento de caballos
    for c in n_caballos
        pid[c] = fork();
        # Comprobaciones...
        # Rutina proceso hijo
            if(!rutina_caballo())
                exit(EXIT_SUCCESS)
            else
                exit(EXIT_FAILURE);
            
    # Rutina proceso padre. NOTA, ¿y si el padre recibe Ctrl-C aquí?
    while(!rutina_etapa_carrera(n_caballos, distancia, ...));
    
    for c in n_caballos
        wait(pid[c]);
        # Comprobaciones… ¿y si un caballo muere inesperadamente? Usar macros P1.
    
    free(pid)
    */
   
    
    for(i = 0; i < params->numCaballos; i++){
        wait(&params->pidsCaballos[i]);
        /*if (params->pidsCaballos[i] == )
        
         * 1. WIFEXITED(status): child exited normally
• WEXITSTATUS(status): return code when child exits

2. WIFSIGNALED(status): child exited because a signal was not caught
• WTERMSIG(status): gives the number of the terminating signal

3. WIFSTOPPED(status): child is stopped
• WSTOPSIG(status): gives the number of the stop signal
         */
    }
    
    for(i = 0; i < params->numCaballos; i++){
        for(j = 0; j < 2; j++)
            free(params->pipesCaballos[i][j]);
        free(params->pipesCaballos[i]);
    }
    
    free(params->pipesCaballos);
    free(params->pidsCaballos);
    
    if (Borrar_Memoria_Compartida( (void*) memCompartida, id_zone) == ERROR)
        exit(EXIT_FAILURE);
    
    free(params);
    
    if (Borrar_Semaforo(mutex_MC) == ERROR){
    	perror("Error borrando el semaforo mutex_MC");
        exit(EXIT_FAILURE);
    }
    
    return (EXIT_SUCCESS);
}