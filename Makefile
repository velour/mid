CC=clang
LD=clang
LDFLAGS=-lSDL -lSDLmain -L/usr/local/lib
CFLAGS=-Wall -Werror -std=c99 $(INCLUDE)

all: mid

mid: main.o
	$(LD) $(LDFLAGS) -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -f mid *.o
