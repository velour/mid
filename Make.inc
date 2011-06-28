ifndef CC
CC := clang
endif

ifndef LD
LD := clang
endif

ifndef AR
AR := ar
endif

MANDCFLAGS := -g -O0 -Wall -Werror -std=c99 -D_POSIX_SOURCE -D_POSIX_C_SOURCE=200112L
MANDLDFLAGS := -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf
