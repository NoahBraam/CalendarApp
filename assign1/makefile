CC = gcc
CFLAGS = -Wall -g -std=c11
LIBS = -Lbin/


all: list parser

list: src/LinkedListAPI.c include/LinkedListAPI.h
	$(CC) -c $(CFLAGS) -fPIC src/LinkedListAPI.c -o bin/LinkedListAPI.o -I include
	$(CC) bin/LinkedListAPI.o -shared -o bin/liblist.so

parser: src/CalendarParser.c include/CalendarParser.h include/LinkedListAPI.h include/CalendarUtils.h src/CalendarUtils.c
	$(CC) -c $(CFLAGS) -fPIC src/CalendarParser.c -o bin/CalendarParser.o -I include
	$(CC) -c $(CFLAGS) -fPIC src/CalendarUtils.c -o bin/CalendarUtils.o -I include
	$(CC) bin/CalendarParser.o bin/CalendarUtils.o -shared -o bin/libcal.so

test: list parser
	$(CC) -c $(CFLAGS) src/testmain.c -o bin/testmain.o -I include
	$(CC) $(CFLAGS) $(LIBS) -Wl,-R -Wl,bin/ -o bin/testmain bin/testmain.o -lcal -llist
clean:
	rm -rf bin/*.o bin/*.so
