# make rule primaria con dummy target ‘all’--> non crea alcun file all ma fa un complete build
# che dipende dai target client e server scritti sotto
all: client server
# make rule per il client
client: client.c utils.c costanti.h
	gcc -o client client.c -Wall
# make rule per il server
server: server.o costanti.h
	gcc -o server server.c -Wall
# pulizia dei file della compilazione (eseguito con ‘make clean’ da terminale)
clean:
	rm *o client server