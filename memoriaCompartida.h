#ifndef memoria
#define memoria
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>
#include <errno.h>
#include <sys/shm.h> 
#include <unistd.h>

void* Crear_Memoria_Compartida(void* path, int* id_zone, int size);
								
int Borrar_Memoria_Compartida(void* buf, int id_zone);

#endif