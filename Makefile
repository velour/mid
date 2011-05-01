CC=clang

all: mid

mid: main.c
	$(CC) -L/usr/local/lib -lSDL -lSDLmain -Wall -Werror -o $@ $<

clean:
	rm -f mid
