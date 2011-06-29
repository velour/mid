LIB := $(LIB) $(shell echo $(HFILES) | \
	sed -n 's|include/([a-z]+)\.h|lib/\1/\1\.a|gp' | \
	sed 's|[a-z]+\.h||g')

$(TARG): $(OFILES)
	@echo ld -o $@ $^ $(LDFLAGS)
	@$(LD) -o $@ $(MANDLDFLAGS) $(LDFLAGS) $^ $(LIB)

$(OFILES): $(HFILES) $(LIB)

%.o: %.c
	@echo cc $< $(CFLAGS)
	@$(CC) -c $(MANDCFLAGS) $(CFLAGS) $<

.PHONY: clean install

clean:
	rm -f *.o $(TARG)

install: $(TARG)
	cp $(TARG) /usr/local/bin
