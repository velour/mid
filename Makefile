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
SDLVER := 13
endif

MANDCFLAGS := -g -O2 -Wall -Werror -std=c99 -D_POSIX_SOURCE -D_POSIX_C_SOURCE=200112L
MANDLDFLAGS := -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf -lm

ifeq ($(OS),win)
MANDCFLAGS += -Dmain=SDL_main
MANDLDFLAGS += -L/mingw/bin $(shell sdl-config --static-libs)
else ifeq ($(OS),Darwin)
OS := osx
MANDLDFLAGS += -framework Foundation
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

ifeq ($(shell uname),Darwin)
installer: all
	mkdir -p Mid.app/Contents/MacOS
	mkdir -p Mid.app/Contents/Resources
	mkdir -p Mid.app/Contents/Frameworks
	cp osx/Info.plist Mid.app/Contents/
	for c in mid lvlgen itmgen enmgen envgen; do cp cmd/$$c/$$c Mid.app/Contents/MacOS/; done
	cp -r resrc/ Mid.app/Contents/Resources/
	for lib in SDL-1.3.0 SDL_image-1.2.0 SDL_mixer-1.2.0 SDL_ttf-2.0.0; do \
		cp /usr/local/lib/lib$$lib.dylib Mid.app/Contents/Frameworks; \
		install_name_tool -id "@executable_path/../Frameworks/lib$$lib.dylib" Mid.app/Contents/Frameworks/lib$$lib.dylib; \
		install_name_tool -change /usr/local/lib/libSDL-1.3.0.dylib "@executable_path/../Frameworks/libSDL-1.3.0.dylib" Mid.app/Contents/Frameworks/lib$$lib.dylib; \
		for c in mid lvlgen itmgen enmgen envgen; do \
			install_name_tool -change /usr/local/lib/lib$$lib.dylib "@executable_path/../Frameworks/lib$$lib.dylib" Mid.app/Contents/MacOS/$$c; \
		done; \
	done
endif
