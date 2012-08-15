# // Copyright © 2011 Steve McCoy and Ethan Burns
# // Licensed under the MIT License. See LICENSE for details.

T := $(TARG)

TARG := cmd/$(T)/$(T)

ALL += $(TARG)

OFILES := $(OFILES:%=cmd/$(T)/%)

ALLO += $(OFILES)

LIBHFILES := $(LIBDEPS:%=include/%.h)

LIBDEPS := $(shell echo $(LIBDEPS) | awk '{ for(i = 1; i <= NF; i++) printf("lib/%s/%s.a ", $$i, $$i) }')

HFILES := $(HFILES:%=cmd/$(T)/%) $(LIBHFILES)

$(TARG): $(OFILES) $(LIBDEPS)
	@echo ld -o $@ $(LDFLAGS) $^
	$(LD) -o $@ $^ $(MANDLDFLAGS) $(LDFLAGS)

$(OFILES): $(HFILES) $(LIBHFILES)
