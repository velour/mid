ifndef CC
CC := clang
endif

ifndef LD
LD := clang
endif

ifndef AR
AR := ar
endif

MANDCFLAGS := -Wall -Werror -std=c99
MANDLDFLAGS :=

ifdef CFLAGS
CFLAGS += $(MANDCFLAGS)
else
CFLAGS := $(MANDCFLAGS)
endif

ifdef LDFLAGS
LDFLAGS += $(MANDLDFLAGS)
else
LDFLAGS := $(MANDLDFLAGS)
endif
