CC=gcc
CFLAGS=-W -Wall -g
SOURCES=20141578.c\
				addr.c\
				assemble.c\
				assemble_command.c\
				debug.c\
				error.c\
				file.c\
				loader.c\
				memory.c\
				opcode.c\
				run.c\
				shell_command.c\
				string_process.c\
				symbol.c

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=20141578.out

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@

.c.o:
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -rf *.o *.out *.lst
