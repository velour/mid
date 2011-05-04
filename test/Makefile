DIRS :=\
	libmid\

TESTDIRS := $(DIRS:%=%.test)

test: $(TESTDIRS)

%.test:
	$(MAKE) -C $*
