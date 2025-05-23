# Взято с https://habr.com/ru/articles/155201/
CC=gcc
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=big_task.c lodepng/lodepng.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=big_task.exe

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm -rf *.o $(EXECUTABLE)