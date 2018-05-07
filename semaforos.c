#include "semaforos.h"

#define OK 0
#define ERROR -1

int Inicializar_Semaforo(int semid, unsigned short *array);
int Borrar_Semaforo(int semid);
int Crear_Semaforo(key_t key, int size, int *semid);
int Down_Semaforo(int id, int num_sem, int undo);
int DownMultiple_Semaforo(int id,int size,int undo,int *active);
int Up_Semaforo(int id, int num_sem, int undo);
int UpMultiple_Semaforo(int id,int size, int undo, int *active);

/***************************************************************
Nombre:
Inicializar_Semaforo.
Descripcion:
Inicializa los semaforos indicados.
Entrada:
 int semid: Identificador del semaforo.
 unsigned short *array: Valores iniciales.
Salida:
int: OK si todo fue correcto, ERROR en caso de error.
************************************************************/
int Inicializar_Semaforo(int semid, unsigned short *array){
    
    union semun{
        int val;
        struct semid_ds* semstats;
        unsigned short* array;
    } args;

    if (semid < 0 || array == NULL) {
        printf("Error en inicializacion del semaforo\n");
        return ERROR;
    }

    args.array = array;
    
    if(semctl(semid,0,SETALL, args) == ERROR){
        printf("Error en inicializacion del semaforo\n");
        return ERROR;
    }
    
    return OK;
    }       

/***************************************************************
Nombre: Borrar_Semaforo.
Descripcion: Borra un semaforo.
Entrada:
 int semid: Identificador del semaforo.
Salida:
int: OK si todo fue correcto, ERROR en caso de error.
***************************************************************/
int Borrar_Semaforo(int semid){
if (semctl(semid, 0, IPC_RMID) < 0)
    return ERROR;

return OK;
}

/***************************************************************
Nombre: Crear_Semaforo.
Descripcion: Crea un semaforo con la clave y el tamaño
especificado. Lo inicializa a 0.
Entrada:
 key_t key: Clave precompartida del semaforo.
 int size: Tamaño del semaforo.
Salida:
 int *semid: Identificador del semaforo.
 int: ERROR en caso de error, 
 0 si ha creado el semaforo,
 1 si ya estaba creado.
**************************************************************/
int Crear_Semaforo(key_t key, int size, int *semid){
	unsigned short* arraySemaforos;
    int i, errno;
    
	*semid = semget(key,size, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
	if((*semid == -1) && (errno == EEXIST)){
	  return 1;
	}
	else if(*semid==-1){
	  return ERROR;
	}
    
    arraySemaforos = (unsigned short*) malloc(sizeof(unsigned short) * size);
    
    for(i = 0; i < size; i++)
        arraySemaforos[i] = 0;
    
    if(Inicializar_Semaforo(*semid, arraySemaforos) == ERROR){
	    free(arraySemaforos);
	    return ERROR;
	}
	
	free(arraySemaforos);
	return OK;
}

/**************************************************************
Nombre:Down_Semaforo
Descripcion: Baja el semaforo indicado
Entrada:
 int semid: Identificador del semaforo.
 int num_sem: Semaforo dentro del array.
 int undo: Flag de modo persistente pese a finalización
abrupta.
Salida:
 int: OK si todo fue correcto, ERROR en caso de error.
***************************************************************/
int Down_Semaforo(int id, int num_sem, int undo){
        struct sembuf sem_oper;
        sem_oper.sem_num = num_sem; /* Actuamos sobre el semáforo 0 de la lista */
	    sem_oper.sem_op = -1; /* Decrementar en 1 el valor del semáforo */
	    sem_oper.sem_flg = undo; /* Para evitar interbloqueos si un proceso acaba inesperadamente */
	    if(semop(id, &sem_oper, 1) == ERROR){
	        return ERROR;
	    }
	    return OK;
    }

/**************************************************************
Nombre:Up_Semaforo
Descripcion: Sube el semaforo indicado
Entrada:
 int semid: Identificador del semaforo.
 int num_sem: Semaforo dentro del array.
 int undo: Flag de modo persistente pese a finalizacion
 abupta.
Salida:
 int: OK si todo fue correcto, ERROR en caso de error.
***************************************************************/
int Up_Semaforo(int id, int num_sem, int undo){
    struct sembuf sem_oper;

    sem_oper.sem_num = num_sem; /* Actuamos sobre el semáforo 0 de la lista */
	sem_oper.sem_op = 1;       /* Decrementar en 1 el valor del semáforo */
	sem_oper.sem_flg = undo;    /* Para evitar interbloqueos si un proceso acaba inesperadamente */

	if(semop(id, &sem_oper, 1) == ERROR){
	    return ERROR;
	}
	return OK;
}

int UpMultiple_Semaforo(int id,int size, int undo, int *active){    
   	int i;

	for(i = 0; i < size; i++){
		if (active[i] == 1){
			if(Up_Semaforo(id, i, undo) == ERROR)
				return ERROR;
		}
	}
	return OK;
}

int DownMultiple_Semaforo(int id,int size,int undo,int *active){
	int i;

	for(i = 0; i < size; i++){
		if(active[i] == 1){
			if(Down_Semaforo(id, i, undo) == ERROR)
				return ERROR;
		}
	}
	return OK;
}

