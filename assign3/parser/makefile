CC = gcc
CFLAGS = -Wall -g -std=c11
LIBS = -Lbin/


all: list parser

list: src/LinkedListAPI.c include/LinkedListAPI.h
	$(CC) -c $(CFLAGS) -fPIC src/LinkedListAPI.c -o LinkedListAPI.o -I include
	$(CC) LinkedListAPI.o -shared -o ../liblist.so

parser: src/CalendarParser.c include/CalendarParser.h include/LinkedListAPI.h include/CalendarUtils.h src/CalendarUtils.c
	$(CC) -c $(CFLAGS) -fPIC src/CalendarParser.c -o CalendarParser.o -I include
	$(CC) -c $(CFLAGS) -fPIC src/CalendarUtils.c -o CalendarUtils.o -I include
	$(CC) CalendarParser.o CalendarUtils.o -shared -o ../libcal.so

clean:
	rm -rf bin/*.o bin/*.so
