CFLAGS=-Wall -Wextra -Werror -pedantic -std=c99 -I./include/

CC=gcc $(CFLAGS)

default: app

build:
	mkdir build

map: src/map.c build
	$(CC) -c src/map.c -o build/map.o

main: src/main.c build
	$(CC) -c src/main.c -o build/main.o

api: src/api.c build
	$(CC) -c src/api.c -o build/api.o

app: map main api
	$(CC) build/map.o build/main.o build/api.o -o app

clean:
	rm -rf build
	rm -f app
