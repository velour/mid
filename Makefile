# © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.

UNAME := $(shell uname)
OS := $(shell echo $(UNAME) | sed 's/.*MINGW.*/win/')

ifndef CC
CC := clang
endif

ifndef LD
LD := clang
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
MANDCFLAGS += $(shell pkg-config --cflags sdl2 SDL2_mixer SDL2_image SDL2_ttf)
else
MANDCFLAGS += $(shell pkg-config --cflags sdl SDL_mixer SDL_image SDL_ttf)
endif

MANDLDFLAGS += \
	-lm \

ifeq ($(SDLVER),2)
MANDLDFLAGS += $(shell pkg-config --libs sdl2 SDL2_mixer SDL2_image SDL2_ttf)
else
MANDLDFLAGS += $(shell pkg-config --libs sdl SDL_mixer SDL_image SDL_ttf)
endif

endif

.PHONY: all clean install env
.DEFAULT_GOAL := all
ALL :=
ALLO :=

CMDS := $(wildcard cmd/*)
LIBS := $(wildcard lib/*)
include $(CMDS:%=%/Make.me)
include $(LIBS:%=%/Make.me)

all: $(ALL)

%.o: %.c
	@echo cc $< $(CFLAGS)
	@$(CC) -c $(MANDCFLAGS) $(CFLAGS) -o $@ $<

clean:
	rm -f $(ALL)
	rm -f $(ALLO)
	rm -f $(shell find . -name '*.exe')

env:
	@echo OS: $(OS)
	@echo CC: $(CC)
	@echo LD: $(LD)
	@echo AR: $(AR)
	@echo SDLVER: $(SDLVER)
	@echo MANDCFLAGS: $(MANDCFLAGS)
	@echo MANDLDFLAGS: $(MANDLDFLAGS)

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
