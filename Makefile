# © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.

UNAME := $(shell uname)
OS := $(shell echo $(UNAME) | sed 's/.*MINGW.*/win/')

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
CMDS +=\
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
SDLVER := 2
endif

MANDCFLAGS := -g -O2 -Wall -Werror -std=c99 -D_POSIX_SOURCE -D_POSIX_C_SOURCE=200112L
MANDLDFLAGS := 

ifeq ($(OS),win)
MANDCFLAGS += -Dmain=SDL_main

MANDLDFLAGS += \
	-L/mingw/bin \
	$(shell sdl-config --static-libs) \
	-lSDL -lSDL_image -lSDL_mixer -lSDL_ttf \
	-lm \

else ifeq ($(OS),Darwin)
OS := osx
MANDCFLAGS += \
	-I/Library/Frameworks/SDL2.framework/Headers \
	-I/Library/Frameworks/SDL2_image.framework/Headers \
	-I/Library/Frameworks/SDL2_mixer.framework/Headers \
	-I/Library/Frameworks/SDL2_ttf.framework/Headers \

MANDLDFLAGS += \
	-framework Foundation \
	-framework SDL2 \
	-framework SDL2_image \
	-framework SDL2_mixer \
	-framework SDL2_ttf \

else
OS := posix

ifeq ($(SDLVER),2)
MANDCFLAGS += -I/usr/local/include/SDL2
else
MANDCFLAGS += -I/usr/include/SDL
endif

MANDLDFLAGS += \
	-lm \

ifeq ($(SDLVER),2)
MANDLDFLAGS += \
	-L/usr/local/lib \
	-lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
else
MANDLDFLAGS += -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf
endif

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

ifeq ($(shell uname),Darwin)
installer: all
	mkdir -p Mid.app/Contents/MacOS
	mkdir -p Mid.app/Contents/Resources
	mkdir -p Mid.app/Contents/Frameworks
	cp osx/Info.plist Mid.app/Contents/
	for c in mid lvlgen itmgen enmgen envgen; do cp cmd/$$c/$$c Mid.app/Contents/MacOS/; done
	cp -r resrc/ Mid.app/Contents/Resources/
	for lib in SDL2 SDL2_image SDL2_mixer SDL2_ttf; do \
		cp -r /Library/Frameworks/$$lib.framework Mid.app/Contents/Frameworks; \
	done
endif
