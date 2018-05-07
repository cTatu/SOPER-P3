FLAGS = -g -Wno-misleading-indentation -Wall -lpthread
EXE = ejercicio2solved ejercicio2planteado ejercicio3 ejercicio4 ejercicio5

all: $(EXE)


ejercicio2solved: ejercicio2solved.c semaforos.c semaforos.h memoriaCompartida.c memoriaCompartida.h
	gcc ejercicio2solved.c semaforos.c memoriaCompartida.c $(FLAGS) -o ejercicio2solved
ejercicio2planteado: ejercicio2planteado.c semaforos.c semaforos.h memoriaCompartida.c memoriaCompartida.h
	gcc ejercicio2planteado.c semaforos.c memoriaCompartida.c $(FLAGS) -o ejercicio2planteado
	
ejercicio3: ejercicio3.c semaforos.c semaforos.h memoriaCompartida.c memoriaCompartida.h
	gcc ejercicio3.c semaforos.c memoriaCompartida.c $(FLAGS) -o ejercicio3

ejercicio4: ejercicio4.c
	gcc ejercicio4.c $(FLAGS) -o ejercicio4
	
ejercicio5: cadena_montaje.c
	gcc cadena_montaje.c $(FLAGS) -o cadena_montaje

clean:
	rm ejercicio2solved ejercicio2planteado ejercicio3 ejercicio4 cadena_montaje

