# © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.

T := $(TARG)

TARG := cmd/$(T)/$(T)

ALL += $(TARG)

OFILES := $(OFILES:%=cmd/$(T)/%)

ALLO += $(OFILES)

LIBHFILES := $(LIBDEPS:%=include/%.h)

LIBDEPS := $(shell echo $(LIBDEPS) | awk '{ for(i = 1; i <= NF; i++) printf("lib/%s/%s.a ", $$i, $$i) }')

HFILES := $(HFILES:%=cmd/$(T)/%) $(LIBHFILES)

$(TARG): $(OFILES) $(LIBDEPS)
	@echo ld $@
	@$(LD) -o $@ $^ $(LDFLAGS)

$(OFILES): $(HFILES) $(LIBHFILES)
