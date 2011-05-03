DIRS :=\
	lib\
	cmd\

include Make.dir

.PHONY: test
test:
	$(MAKE) -C test
