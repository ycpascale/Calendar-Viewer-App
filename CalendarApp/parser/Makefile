FLAGS = -Wall -std=c11 -Iinclude -g
CC = gcc

#UNAME Shell Variable
UNAME_S := $(shell uname -s)

# OS X Dependant Variables
ifeq ($(UNAME_S), Darwin)
    LIB := sharedLib.dylib
# GNU/Linux Depedant Variables
else ifeq ($(UNAME_S), Linux)
    LIB := sharedLib.so
endif

$(LIB): CalendarParser.o LinkedListAPI.o
	$(CC) -shared -o ../$(LIB) CalendarParser.o LinkedListAPI.o

all: list parser

LinkedListAPI.o: src/LinkedListAPI.c
	$(CC) $(FLAGS) -c src/LinkedListAPI.c -fPIC -o LinkedListAPI.o

CalendarParser.o: src/CalendarParser.c
	$(CC) $(FLAGS) -c src/CalendarParser.c -fPIC -o CalendarParser.o

list: LinkedListAPI.o
	gcc -shared -o bin/liblist.so LinkedListAPI.o

parser: CalendarParser.o LinkedListAPI.o
	gcc -shared -o ../libcal.so CalendarParser.o LinkedListAPI.o

clean:
	rm -f *.o bin/*
