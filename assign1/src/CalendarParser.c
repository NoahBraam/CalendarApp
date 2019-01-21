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
#include "CalendarUtils.h"

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

  char cur;
  const char tok[2] = ":";
  int lineStart = 0, lineEnd = 0;
  bool endCal = false;
  bool creatingEvent = false, creatingAlarm = false;
  Calendar* tmpCal = NULL;
  Event* tmpEvent;
  while ((cur = fgetc(fp)) != EOF) {
    // Make sure calendar isn't done
    if (endCal) {
      *obj = NULL;
      err = INV_FILE;
      return err;
    }
    // Start Read Line //
    fseek(fp, lineEnd, SEEK_SET);
    lineStart = ftell(fp);
    lineEnd = readLine(fp);
    int len = lineEnd - lineStart;

    char* line = malloc(sizeof(char) * len);

    fseek(fp, lineStart, SEEK_SET);
    fgets(line, len, fp);

    line = fixLine(line);
    printf("%s\n", line);
    fseek(fp, lineEnd, SEEK_SET);
    // ======== End Read Line ======== //

    // ======== Start Handle Line ======== //
    if (startsWith(line, "BEGIN:")) {
      if (endsWith(line, "VCALENDAR")) {
        if (tmpCal == NULL) {
          tmpCal = initCal(&printEvent, &deleteEvent, &compareEvents, &printProperty, &deleteProperty, &compareProperties);
        } else {
          // TODO: actual error code.
          *obj = NULL;
          err = INV_FILE;
          return err;
        }
      } else if (endsWith(line, "VEVENT")) {
        tmpEvent = initEvent(&printProperty, &deleteProperty, &compareProperties, &printAlarm, &deleteAlarm, &compareAlarms);
        creatingEvent = true;
      } else {

      }

    } else if (startsWith(line, "END:")) {
      if (endsWith(line, "VCALENDAR")) {
        endCal = true;
      } else if (endsWith(line, "VEVENT")) {
        insertFront(tmpCal->events, tmpEvent);
        creatingEvent = false;
      } else {

      }
    } else if (startsWith(line, "VERSION:")) {
      if (tmpCal == NULL) {
        // TODO: real error code
        *obj = NULL;
        err = INV_FILE;
        return err;
      }
      if (tmpCal->version == 0.0) {
        char* version = strtok(line, tok);
        version = strtok(NULL, tok);
        tmpCal->version = atof(version);
      } else {
        deleteCalendar(tmpCal);
        // TODO: real error code
        *obj = NULL;
        err = INV_FILE;
        return err;
      }
    } else if (startsWith(line, "PRODID:")) {
      if (tmpCal == NULL) {
        // TODO: error code
        *obj = NULL;
        err = INV_FILE;
        return err;
      }
      if (strcmp(tmpCal->prodID, "temp") == 0) {
        char* prodID = strtok(line, tok);
        prodID = strtok(NULL, tok);
        strcpy(tmpCal->prodID, prodID);
      } else {
        deleteCalendar(tmpCal);
        *obj=NULL;
        // TODO: error codes
        err = INV_FILE;
        return err;
      }
    } else {
      // default extra property...
      if (tmpCal == NULL) {
        // TODO: error code
        *obj = NULL;
        err = INV_FILE;
        return err;
      }
      Property* tmpProp = createProperty(line);
      insertFront(tmpCal->properties, tmpProp);
    }
    free(line);
  }
  fclose(fp);
  *obj = tmpCal;
  err = OK;
  return err;
}

void deleteCalendar(Calendar* obj) {
  freeList(obj->properties);
  freeList(obj->events);
  free(obj);
}

char* printCalendar(const Calendar* obj) {
  //char* evtStr = toString(obj->events);
  int len = 100;
  char* propStr = toString(obj->properties);
  int propLen = strlen(propStr);
  len+=propLen;

  //char* str = malloc(sizeof(char) * (strlen(obj->prodID) + strlen(evtStr) + strlen(propStr) + 50));
  //int len = sizeof(str);

  //snprintf(str, len, "Version: %lf ID: %s Events: %s Props: %s", obj->version, obj->prodID, evtStr, propStr);
  char* str = malloc(sizeof(char) * len);
  snprintf(str, len, "Version: %.2lf\nProdID: %s\nProperties: %s", obj->version, obj->prodID, propStr);
  //free(evtStr);
  free(propStr);

  return str;
}

char* printError(ICalErrorCode err) {
  if (err == OK) {
    char* str = malloc(sizeof(char) * 4);
    strcpy(str, "OK\n");
    return str;
  } else {
    char* str = malloc(sizeof(char) * 13);
    strcpy(str, "Invalid File\n");
    return str;
  }
}

//============ Helper Functions =============//
void deleteEvent(void* toBeDeleted) {
  if (toBeDeleted == NULL) {
    return;
  }
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
  if (toBePrinted == NULL) {
    return NULL;
  }
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
  if (toBeDeleted == NULL) {
    return;
  }
  Alarm* alarm = (Alarm*)toBeDeleted;
  free(alarm->trigger);
  freeList(alarm->properties);
  free(alarm);
}

int compareAlarms(const void* first, const void* second) {
  return 1;
}

char* printAlarm(void* toBePrinted) {
  if (toBePrinted == NULL) {
    return NULL;
  }
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
  if (toBeDeleted == NULL) {
    return;
  }
  Property* prop = (Property*)toBeDeleted;
  free(prop);
}

int compareProperties(const void* first, const void* second) {
  return 1;
}

char* printProperty(void* toBePrinted) {
  if (toBePrinted == NULL) {
    return NULL;
  }
  Property* prop = (Property*)toBePrinted;
  char* str = malloc(sizeof(char) * (strlen(prop->propName) + strlen(prop->propDescr) + 4));
  strcpy(str, prop->propName);
  strcat(str, ": ");
  strcat(str, prop->propDescr);
  strcat(str, "\n");
  return str;
}

void deleteDate(void* toBeDeleted) {
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
