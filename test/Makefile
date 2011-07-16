# // Copyright © 2011 Steve McCoy and Ethan Burns
# // Licensed under the MIT License. See LICENSE for details.
DIRS :=\
	libmid\
	librng\

TESTDIRS := $(DIRS:%=%.test)

test: $(TESTDIRS)

%.test:
	$(MAKE) -C $*
