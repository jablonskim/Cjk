CC=gcc
C_FLAGS=-Wall -Werror -ansi -pedantic -g
L_FLAGS=-lpthread -lm

.PHONY: all clean

all: client server vehicle

client: client.o utils.o
	${CC} -o client client.o utils.o ${L_FLAGS}

server: server.o utils.o vehicle_list.o
	${CC} -o server server.o utils.o vehicle_list.o ${L_FLAGS}

vehicle: vehicle.o utils.o
	${CC} -o vehicle vehicle.o utils.o ${L_FLAGS}

client.o: client.c utils.h
	${CC} -o client.o -c client.c ${C_FLAGS}

server.o: server.c utils.h vehicle_list.h server.h
	${CC} -o server.o -c server.c ${C_FLAGS}

vehicle.o: vehicle.c utils.h vehicle.h
	${CC} -o vehicle.o -c vehicle.c ${C_FLAGS}

utils.o: utils.c utils.h
	${CC} -o utils.o -c utils.c ${C_FLAGS}

vehicle_list.o: vehicle_list.c vehicle_list.h utils.h
	${CC} -o vehicle_list.o -c vehicle_list.c ${C_FLAGS}

clean:
	-rm -f *.o server client vehicle
