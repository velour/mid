CMDS :=\
	mid\
	enmnear\
	envgen\
	envnear\
	itmnear\
	itmgen\
	lvlgen\
	rectview\

LIBS :=\
	mid\
	log\
	rng\

ifndef CC
CC := clang
endif

ifndef LD
LD := clang
endif

ifndef AR
AR := ar
endif

MANDCFLAGS := -g -O2 -Wall -Werror -std=c99 -D_POSIX_SOURCE -D_POSIX_C_SOURCE=200112L
MANDLDFLAGS := -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf

.PHONY: all clean install
.DEFAULT_GOAL := all
ALL :=
ALLO :=

include $(CMDS:%=cmd/%/Makefile)
include $(LIBS:%=lib/%/Makefile)

all: $(ALL)

%.o: %.c
	@echo cc $< $(CFLAGS)
	@$(CC) -c $(MANDCFLAGS) $(CFLAGS) -o $@ $<

clean:
	rm -f $(ALL)
	rm -f $(ALLO)