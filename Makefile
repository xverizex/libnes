test:
	make -f tests/Makefile
all:
	make -f Makefile.linux
clean:
	make clean -f Makefile.linux
