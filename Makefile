# // Copyright © 2011 Steve McCoy and Ethan Burns
# // Licensed under the MIT License. See LICENSE for details.
CMDS :=\
	mid\
	enmgen\
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
CC := clang -fno-color-diagnostics
endif

ifndef LD
LD := clang -fno-color-diagnostics
endif

ifndef AR
AR := ar
endif

ifndef SDLVER
SDLVER := 13
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
