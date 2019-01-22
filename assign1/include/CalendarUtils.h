/**
 * Noah Braam
 * 0960202
 */
#ifndef CALENDARUTILS_H
#define CALENDARUTILS_H

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "CalendarParser.h"

// ======== Init Functions ======== //
Calendar* initCal(char* (*printFunction1)(void* toBePrinted),void (*deleteFunction1)(void* toBeDeleted),int (*compareFunction1)(const void* first,const void* second), char* (*printFunction2)(void* toBePrinted),void (*deleteFunction2)(void* toBeDeleted),int (*compareFunction2)(const void* first,const void* second));
Event* initEvent(char* (*printFunction1)(void* toBePrinted),void (*deleteFunction1)(void* toBeDeleted),int (*compareFunction1)(const void* first,const void* second), char* (*printFunction2)(void* toBePrinted),void (*deleteFunction2)(void* toBeDeleted),int (*compareFunction2)(const void* first,const void* second));
Property* createProperty(char* line);
void handleDTStamp(char* dt, DateTime* toChange);

char* readLine(FILE* fp);
bool startsWith(char* str, char* search);
bool endsWith(char* str, char* search);

#endif
