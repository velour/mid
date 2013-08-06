DIRS :=\
	libmid\
	librng\

TESTDIRS := $(DIRS:%=%.test)

test: $(TESTDIRS)

%.test:
	$(MAKE) -C $*
