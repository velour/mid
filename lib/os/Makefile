include Make.inc

TARG := os.a

OFILES :=\
	dir_$(OS).o\
	pipe_$(OS).o\
	appdata_$(OS).o\

HFILES :=\

lib/os/appdata_osx.o: lib/os/appdata_osx.m
	@echo occ $< $(CFLAGS)
	@$(CC) -c $(MANDCFLAGS) $(CFLAGS) -o $@ $<

include Make.lib
