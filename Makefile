CMDS :=\
	mid\

LIBS :=\
	mid\
	log\
	rng\

.PHONY: all clean install
.DEFAULT_GOAL := all
ALL :=
ALLO :=

include $(CMDS:%=cmd/%/Makefile)

include $(LIBS:%=lib/%/Makefile)

all: $(ALL)

%.o: %.c
	@echo cc $< $(CFLAGS)
	@$(CC) -c $(MANDCFLAGS) $(CFLAGS) $<

clean:
	rm -f $(ALL)
	rm -f $(ALLO)