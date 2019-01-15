CC = gcc
CFLAGS = -Wall -g -std=c11 -fPIC
LIBS = -Lbin/


all: list parser

list: src/LinkedListAPI.c include/LinkedListAPI.h
	$(CC) -c $(CFLAGS) src/LinkedListAPI.c -o bin/LinkedListAPI.o -I include
	$(CC) bin/LinkedListAPI.o -shared -o bin/liblist.so

parser: src/CalendarParser.c include/CalendarParser.h include/LinkedListAPI.h
	$(CC) -c $(CFLAGS) src/CalendarParser.c -o bin/CalendarParser.o -I include
	$(CC) bin/CalendarParser.o -shared -o bin/libcal.so

clean:
	rm -rf bin/*.o bin/*.so
