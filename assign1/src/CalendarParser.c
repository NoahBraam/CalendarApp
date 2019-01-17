/**
 * Noah Braam
 * 0960202
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "CalendarParser.h"
#include "LinkedListAPI.h"

ICalErrorCode createCalendar(char* fileName, Calendar** obj) {
  ICalErrorCode err;

  // Filename is NULL or too short
  if (fileName == NULL || strlen(fileName) < 5) {
    *obj = NULL;
    err = INV_FILE;
    return err;
  }

  // Wrong extention
  if(strcmp(fileName + strlen(fileName) - 4, ".ics")) {
    *obj = NULL;
    err = INV_FILE;
    return err;
  }

  FILE* fp = fopen(fileName, "r");

  // File does not exist
  if (fp == NULL){
    *obj = NULL;
    err = INV_FILE;
    return err;
  }

  err = OK;
  return err;
}

void deleteCalendar(Calendar* obj) {
  freeList(obj->properties);
  freeList(obj->events);
  free(obj);
}

char* printError(ICalErrorCode err) {
  if (err == OK) {
    char* str = malloc(sizeof(char) * 3);
    strcpy(str, "OK");
    return str;
  } else {
    char* str = malloc(sizeof(char) * 13);
    strcpy(str, "Invalid File");
    return str;
  }
}

//============ Helper Functions =============//
void deleteEvent(void* toBeDeleted) {
  Event* evt = (Event*)toBeDeleted;
  deleteDate(&evt->creationDateTime);
  deleteDate(&evt->startDateTime);
  freeList(evt->properties);
  freeList(evt->alarms);
  free(evt);
}

int compareEvents(const void* first, const void* second) {
  return 1;
}

char* printEvent(void* toBePrinted) {
  Event* evt = (Event*)toBePrinted;

  char* createTime = printDate(&evt->creationDateTime);
  char* startTime = printDate(&evt->startDateTime);
  char* propStr = toString(evt->properties);
  char* alarmStr = toString(evt->alarms);

  char* str = malloc(sizeof(char) * (strlen(createTime) + strlen(startTime) + strlen(propStr) + strlen(alarmStr) + strlen(evt->UID) + 50));

  strcpy(str, "UID: ");
  strcat(str, evt->UID);
  strcat(str, " Created: ");
  strcat(str, createTime);
  strcat(str, " Start: ");
  strcat(str, startTime);
  strcat(str, " Properties: ");
  strcat(str, propStr);
  strcat(str, " Alarms: ");
  strcat(str, alarmStr);

  free(createTime);
  free(startTime);
  free(propStr);
  free(alarmStr);

  return str;
}

void deleteAlarm(void* toBeDeleted) {
  Alarm* alarm = (Alarm*)toBeDeleted;
  free(alarm->trigger);
  freeList(alarm->properties);
  free(alarm);
}

int compareAlarms(const void* first, const void* second) {
  return 1;
}

char* printAlarm(void* toBePrinted) {
  Alarm* alarm = (Alarm*)toBePrinted;
  char* propStr = toString(alarm->properties);

  char* str = malloc(sizeof(char) * (strlen(propStr) + strlen(alarm->action) + strlen(alarm->trigger) + 25));
  strcpy(str, "Action: ");
  strcat(str, alarm->action);
  strcat(str, " Trigger: ");
  strcat(str, alarm->trigger);
  strcat(str, " Props: ");
  strcat(str, propStr);

  free(propStr);
  return str;
}

void deleteProperty(void* toBeDeleted) {
  Property* prop = (Property*)toBeDeleted;
  free(prop->propDescr);
  free(prop);
}

int compareProperties(const void* first, const void* second) {
  return 1;
}

char* printProperty(void* toBePrinted) {
  Property* prop = (Property*)toBePrinted;
  char* str = malloc(sizeof(char) * (strlen(prop->propName) + strlen(prop->propDescr) + 4));
  strcpy(str, prop->propName);
  strcat(str, ": ");
  strcat(str, prop->propDescr);
  return str;
}

void deleteDate(void* toBeDeleted) {
  free(toBeDeleted);
}

int compareDates(const void* first, const void* second) {
  return 1;
}

char* printDate(void* toBePrinted) {
  if (toBePrinted == NULL) {
    return NULL;
  }
  DateTime* dt = (DateTime*)toBePrinted;
  char* print = malloc(sizeof(char) * 23);
  strcat(print, dt->date);
  strcat(print, dt->time);

  if (dt->UTC) {
    strcat(print, "true");
  } else {
    strcat(print, "false");
  }

  return print;
}
