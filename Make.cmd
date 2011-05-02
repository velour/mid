$(TARG): $(OFILES)
	quietld -o $@ $(LDFLAGS) $^ $(LIB)

$(OFILES): $(HFILES) $(LIB)

%.o: %.c
	quietc $(CFLAGS) $<

.PHONY: clean install

clean:
	rm -f *.o $(TARG)

install: $(TARG)
	cp $(TARG) /usr/local/bin
