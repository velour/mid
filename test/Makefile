# © 2013 the Mid Authors under the MIT license. See AUTHORS for the list of authors.
DIRS :=\
	libmid\
	librng\

TESTDIRS := $(DIRS:%=%.test)

test: $(TESTDIRS)

%.test:
	$(MAKE) -C $*
