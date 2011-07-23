# // Copyright © 2011 Steve McCoy and Ethan Burns
# // Licensed under the MIT License. See LICENSE for details.

UNAME := $(shell uname)
OS := $(shell echo $(UNAME) | sed 's/.*mingw.*/win/i')

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
ifeq ($(OS),win)
	tee\
endif

LIBS :=\
	mid\
	log\
	rng\
	os\

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

ifeq ($(OS),win)
MANDCFLAGS += -Dmain=SDL_main
MANDLDFLAGS += -L/mingw/bin -L/mingw/lib -lmingw32 -lSDLmain -lm -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lversion -luuid
else
OS := posix
endif

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
	rm -f $(shell find . -name '*.exe')

ifeq ($(OS),win)
installer: all
	mkdir -p Mid
	cp /mingw/bin/SDL*.dll Mid
	for c in mid lvlgen itmgen enmgen envgen tee; do cp cmd/$$c/$$c Mid/; done
	cp -r resrc/ Mid/
endif
