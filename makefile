CC=gcc
CFLAGS=-I.
LDFLAGS=-fPIC -shared
SOURCES=cpunes.c handlers.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=libnes.so
LIB_DIR=/usr/local/lib64

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.c.o:
	@echo [CC] $<
	@cc -c $< $(CFLAGS) $(LDFLAGS) -o $@

clean:
	rm -f *o
	rm -f $(EXECUTABLE)
