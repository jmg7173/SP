CC=gcc
CFLAGS=-W -Wall
SOURCES=20141578.c shell_command.c opcode.c memory.c assemble.c error.c string_process.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=20141578.out

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@

.c.o:
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -rf *.o *.out *.lst *.obj
