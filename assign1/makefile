CC = gcc
CFLAGS = -Wall -g -std=c11
LIBS = -Lbin/


all: list parser

list: src/LinkedListAPI.c include/LinkedListAPI.h
	$(CC) -c $(CFLAGS) src/LinkedListAPI.c -o bin/LinkedListAPI.o
	$(CC) bin/LinkedListAPI.o -shared -o liblist.so

parser: src/CalendarParser.c include/CalendarParser.h include/LinkedListAPI.h
	$(CC) -c $(CFLAGS) src/CalendarParser.c -o bin/CalendarParser.o
	$(CC) bin/CalendarParser.o -shared -o libcal.so

clean:
	rm -rf bin/*.o bin/*.so
