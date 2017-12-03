CFLAGS=-Wall -Wextra -Werror -pedantic -std=c99 -I./include/

CC=gcc $(CFLAGS)

ifeq ($(OS),Windows_NT)
    LINKS =-lws2_32
endif

default: app

build:
	mkdir build

map: src/map.c build
	$(CC) -c src/map.c -o build/map.o

main: src/main.c build
	$(CC) -c src/main.c -o build/main.o

api: src/api.c build
	$(CC) -c src/api.c -o build/api.o ${LINKS}

socklib: src/socklib.c build
	$(CC) -c src/socklib.c -o build/socklib.o ${LINKS}

server: src/server.c build
	$(CC) -c src/server.c -o build/server.o ${LINKS}

client: src/client.c build
	$(CC) -c src/client.c -o build/client.o ${LINKS}

srv-test: server client socklib
	$(CC) build/server.o build/socklib.o build/client.o -o server ${LINKS}

app: map main api
	$(CC) build/map.o build/main.o build/api.o -o app ${LINKS}



server: 

clean:
	rm -rf build
	rm -f app
