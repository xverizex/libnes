CC=gcc
CFLAGS=-I..
LDFLAGS=
SOURCES=test.c ../handlers.c thumb.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=tests
LIB_DIR=/usr/local/lib64

all: $(SOURCES) $(EXECUTABLE)
	./tests

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.c.o:
	@echo [CC] $<
	@cc -c $< $(CFLAGS) $(LDFLAGS) -o $@

clean:
	rm -f *.o
	rm -f $(EXECUTABLE)
