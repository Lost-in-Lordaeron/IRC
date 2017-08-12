all: Server

Server: server.o
	gcc server.o -o Server
server.o: server.c server.h
	gcc -c server.c
clean: 
	rm -rf Server server.o

