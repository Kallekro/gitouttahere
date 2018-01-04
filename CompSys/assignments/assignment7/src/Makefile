CC = gcc
CFLAGS =-Wall -Wextra -pedantic -std=gnu11 -pthread -g3
LD_FLAGS = -lpthread

all : peer name_server

rebuild : clean all

csapp.o : csapp.c csapp.h
	$(CC) $(CFLAGS) -c $< -o $@

peer : peer.c peer.h csapp.o
	$(CC) $(CFLAGS) $< *.o -o $@ $(LD_FLAGS)

name_server : name_server.c name_server.h csapp.o job_queue.c job_queue.h socklib.h
	$(CC) $(CFLAGS) job_queue.c $< *.o -o $@ $(LD_FLAGS)

clean :
	rm -rf *.o peer name_server job_queue

../src.zip :
	make clean
	cd .. && zip src.zip -r src
