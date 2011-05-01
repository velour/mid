CC=clang
INCLUDE=-L/usr/local/lib
LDFLAGS=-lSDL -lSDLmain
CFLAGS=-Wall -Werror -std=c99 $(INCLUDE)

all: mid

mid: main.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f mid
