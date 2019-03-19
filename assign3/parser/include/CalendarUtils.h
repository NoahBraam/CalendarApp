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
Alarm* initAlarm(char* (*printFunction1)(void* toBePrinted),void (*deleteFunction1)(void* toBeDeleted),int (*compareFunction1)(const void* first,const void* second));
Property* createProperty(char* line);
ICalErrorCode handleDTStamp(char* dt, DateTime* toChange);
bool validEvent(Event* evt);
bool validAlarm(Alarm* alarm);
ICalErrorCode validCal(Calendar* cal);
void writeProps(FILE* fp, List* props);
void writeAlarms(FILE* fp, List* alarms);
bool comparePropertiesByName(const void* first, const void* second);
bool allPropsValid(List* properties);
int numPropertiesWithName(List* properties, char* name);

bool validAlarmProperties(List* properties);
bool onlyValidAlarmPropNames(List* properties);

bool validEventProperties(List* properties);
bool onlyValidEventPropNames(List* properties);

bool validCalProperties(List* properties);
bool onlyValidCalPropNames(List* properties);

char* readLine(FILE* fp);
bool startsWith(char* str, char* search);
bool endsWith(char* str, char* search);

char* parseCalReturnJSON(char* file);
char* parseCalReturnEvents(char* file);
char* addEventToFile(char* filename, char* evtJSON, char* createDT, char* startDT, char* summary);
char* newCalendarFile(char* filename, char* calJSON, char* evtJSON, char* createDT, char* startDT, char* summary);


char* propertyToJSON(const Property* prop);
char* propertyListToJSON(const List* propList);
char* alarmToJSON (const Alarm* alm);
char* alarmListToJSON(const List* alarmList);

#endif
