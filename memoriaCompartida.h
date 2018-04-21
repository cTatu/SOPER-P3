#ifndef VARIABLE
#define VARIABLE
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>
#include <errno.h>
#include <sys/shm.h> 

void Crear_Memoria_Compartida(char* buffer, char* path, int* id_zone, int size);
								
int Borrar_Memoria_Compartida(char* buf, int id_zone);

#endif