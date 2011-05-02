$(TARG): $(OFILES)
	quietld $(LDFLAGS) -o $@ $^ $(LIB)

$(OFILES): $(HFILES) $(LIB)

%.o: %.c
	quietc -c $(CFLAGS) -o $@ $<

.PHONY: clean install

clean:
	rm -f *.o $(TARG)

install: $(TARG)
	cp $(TARG) /usr/local/bin
