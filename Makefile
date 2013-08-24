# © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.

OS := $(shell uname | sed 's/.*MINGW.*/win/')

CC := clang
LD := clang
AR := ar

MANDCFLAGS := -g -O2 -Wall -Werror -std=c99
MANDLDFLAGS := 

ifeq ($(OS),win)
MANDCFLAGS += \
	-Dmain=SDL_main \
	-I/mingw/include/SDL2

MANDLDFLAGS += \
	-L/mingw/bin \
	$(shell sdl-config --static-libs) \
	-lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf \
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

MANDCFLAGS += $(shell pkg-config --cflags sdl2 SDL2_mixer SDL2_image SDL2_ttf)

MANDLDFLAGS += \
	-lm \
	$(shell pkg-config --libs sdl2 SDL2_mixer SDL2_image SDL2_ttf) \

endif

override CFLAGS += $(MANDCFLAGS)
override LDFLAGS += $(MANDLDFLAGS)

.PHONY: all clean install env prereqs
.DEFAULT_GOAL := all
ALL :=
ALLO :=

CMDS := $(wildcard cmd/*)
LIBS := $(wildcard lib/*)
include $(CMDS:%=%/Make.me)
include $(LIBS:%=%/Make.me)

all: $(ALL)

%.o: %.c
	@echo cc $<
	@$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -f $(ALL)
	rm -f $(ALLO)
	rm -f $(shell find . -name '*.exe')

env:
	@echo OS: $(OS)
	@echo CC: $(CC)
	@echo LD: $(LD)
	@echo AR: $(AR)
	@echo CFLAGS: $(CFLAGS)
	@echo LDFLAGS: $(LDFLAGS)

prereqs:
	@./$(OS)/getprereqs.sh

ifeq ($(OS),win)
installer: all
	mkdir -p Mid
	cp /mingw/bin/SDL2*.dll Mid
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
