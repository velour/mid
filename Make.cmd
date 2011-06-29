T := $(TARG)

TARG := cmd/$(T)/$(T)

ALL += $(TARG)

OFILES := $(OFILES:%=cmd/$(T)/%)

ALLO += $(OFILES)

LIBHFILES := $(LIBDEPS:%=include/%.h)

LIBDEPS := $(shell echo $(LIBDEPS) | 9 sed 's|([a-z]+)|lib/\1/\1\.a|g')

HFILES := $(HFILES:%=cmd/$(T)/%) $(LIBHFILES)

$(TARG): $(OFILES) $(LIBDEPS)
	@echo ld -o $@ $^ $(LDFLAGS)
	$(LD) -o $@ $(MANDLDFLAGS) $(LDFLAGS) $^

$(OFILES): $(HFILES) $(LIBHFILES)
