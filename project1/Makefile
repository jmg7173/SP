CC=gcc
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=20141578.c shell_command.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=20141578.out

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

