#include <signal.h>
#include <stdbool.h>

#define READ 0
#define CHILD 0
#define WRITE 1
#define PARENT 1
#define OK 0 	
#define ERROR -1

typedef struct{
	unsigned int numCaballos;
    unsigned int longitudCarrera;
    unsigned int numApostadores;
    unsigned int numVentanillas;
    unsigned int cantidadDinero;
    int*** pipesCaballos;
    int* pidsCaballos;
}Parametros;

typedef enum
_Tirada{
    NORMAL, 
    PRIMERO, 
    ULTIMO
}TipoTirada;

int rutina_caballo(int ID);