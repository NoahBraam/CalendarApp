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

  // Set to NULL so that we don't need to if there is an error.
  *obj = NULL;
  // Filename is NULL or too short
  if (fileName == NULL || strlen(fileName) < 5) {
    err = INV_FILE;
    return err;
  }

  // Wrong extention
  if(strcmp(fileName + strlen(fileName) - 4, ".ics")) {
    err = INV_FILE;
    return err;
  }

  FILE* fp = fopen(fileName, "r");

  // File does not exist
  if (fp == NULL){
    err = INV_FILE;
    return err;
  }

  char cur;
  const char tok[3] = ":;";
  bool endCal = false;
  bool creatingEvent = false, creatingAlarm = false;
  Calendar* tmpCal = NULL;
  Event* tmpEvent;
  Alarm* tmpAlarm;
  while ((cur = fgetc(fp)) != EOF) {
    // Make sure calendar isn't done
    if (endCal) {
      err = INV_FILE;
      return err;
    }
    // Start Read Line //
    char* line = readLine(fp);
    if (line == NULL) {
      break;
    }
    // ======== End Read Line ======== //

    // ======== Start Handle Line ======== //
    if (startsWith(line, "BEGIN:")) {
      if (endsWith(line, "VCALENDAR")) {
        if (tmpCal == NULL) {
          tmpCal = initCal(&printEvent, &deleteEvent, &compareEvents, &printProperty, &deleteProperty, &compareProperties);
        } else {
          free(line);
          fclose(fp);
          err = INV_FILE;
          return err;
        }
      } else if (endsWith(line, "VEVENT")) {
        if (creatingEvent) {
          err = INV_EVENT;
          free(line);
          fclose(fp);
          if (creatingAlarm) {
            deleteAlarm(tmpAlarm);
          }
          deleteEvent(tmpEvent);
          deleteCalendar(tmpCal);
          return err;
        }
        tmpEvent = initEvent(&printProperty, &deleteProperty, &compareProperties, &printAlarm, &deleteAlarm, &compareAlarms);
        creatingEvent = true;
      } else if (endsWith(line, "VALARM")) {
        if (creatingEvent && !creatingAlarm) {
          tmpAlarm = initAlarm(&printProperty, &deleteProperty, &compareProperties);
          creatingAlarm = true;
        } else {
          err = INV_ALARM;
          if (creatingAlarm) {
            deleteAlarm(tmpAlarm);
          }
          if (creatingEvent) {
            deleteEvent(tmpEvent);
          }
          deleteCalendar(tmpCal);
          free(line);
          fclose(fp);
          return err;
        }

      } else {
          Property* tmpProp = createProperty(line);
          if (tmpProp == NULL) {
            if (creatingAlarm) {
              err = INV_ALARM;
              deleteAlarm(tmpAlarm);
            }
            if (creatingEvent) {
              if (err != INV_ALARM) {
                err = INV_EVENT;
              }
              deleteEvent(tmpEvent);
            }
            if (err != INV_ALARM && err != INV_EVENT) {
              err = INV_CAL;
            }
            deleteCalendar(tmpCal);
            free(line);
            fclose(fp);
            return err;
          } else if (creatingAlarm) {
            insertBack(tmpAlarm->properties, tmpProp);
          } else if (creatingEvent) {
            insertBack(tmpEvent->properties, tmpProp);
          } else {
            insertBack(tmpCal->properties, tmpProp);
          }
      }
    } else if (startsWith(line, "END:")) {
      if (endsWith(line, "VCALENDAR")) {
        endCal = true;
      } else if (endsWith(line, "VEVENT")) {
        if(!validEvent(tmpEvent)) {
          deleteEvent(tmpEvent);
          deleteCalendar(tmpCal);
          free(line);
          fclose(fp);
          err = INV_EVENT;
          return err;
        } else {
          insertBack(tmpCal->events, tmpEvent);
          creatingEvent = false;
        }

      } else if (endsWith(line, "VALARM")) {
        if (!validAlarm(tmpAlarm)) {
          deleteAlarm(tmpAlarm);
          deleteEvent(tmpEvent);
          deleteCalendar(tmpCal);
          free(line);
          fclose(fp);
          err = INV_ALARM;
          return err;
        } else {
          insertBack(tmpEvent->alarms, tmpAlarm);
          creatingAlarm = false;
        }
      } else {
        Property* tmpProp = createProperty(line);
          if (tmpProp == NULL) {
            if (creatingAlarm) {
              err = INV_ALARM;
              deleteAlarm(tmpAlarm);
            }
            if (creatingEvent) {
              if (err != INV_ALARM) {
                err = INV_EVENT;
              }
              deleteEvent(tmpEvent);
            }
            if (err != INV_ALARM && err != INV_EVENT) {
              err = INV_CAL;
            }
            deleteCalendar(tmpCal);
            free(line);
            fclose(fp);
            return err;
          } else if (creatingAlarm) {
            insertBack(tmpAlarm->properties, tmpProp);
          } else if (creatingEvent) {
            insertBack(tmpEvent->properties, tmpProp);
          } else {
            insertBack(tmpCal->properties, tmpProp);
          }
      }
    } else if (startsWith(line, "VERSION:")) {
      if (tmpCal == NULL) {
        free(line);
        fclose(fp);
        err = INV_FILE;
        return err;
      }
      if (tmpCal->version == 0.0) {
        char* version = strtok(line, tok);
        version = strtok(NULL, tok);
        tmpCal->version = atof(version);
      } else {
        deleteCalendar(tmpCal);
        free(line);
        fclose(fp);
        err = DUP_VER;
        return err;
      }
    } else if (startsWith(line, "PRODID:")) {
      if (tmpCal == NULL) {
        free(line);
        fclose(fp);
        err = INV_CAL;
        return err;
      }
      if (strcmp(tmpCal->prodID, "temp") == 0) {
        char* prodID = strtok(line, tok);
        prodID = strtok(NULL, tok);
        strcpy(tmpCal->prodID, prodID);
      } else {
        free(line);
        fclose(fp);
        deleteCalendar(tmpCal);
        err = DUP_PRODID;
        return err;
      }
    } else if(startsWith(line, ";")) {
      // Comment, do nothing
    } else if(startsWith(line, "UID:")) {
      if (!creatingEvent || strcmp(tmpEvent->UID, "temp") != 0) {
        free(line);
        fclose(fp);
        deleteEvent(tmpEvent);
        deleteCalendar(tmpCal);
        err = INV_EVENT;
        return err;
      } else {
        char* uid = strtok(line, tok);
        uid = strtok(NULL,tok);
        strcpy(tmpEvent->UID, uid);
      }
    } else if (startsWith(line, "DTSTAMP:")) {
      if (!creatingEvent){
        Property* tmpProp = createProperty(line);
          if (tmpProp == NULL) {
            if (creatingAlarm) {
              err = INV_ALARM;
              deleteAlarm(tmpAlarm);
            }
            if (creatingEvent) {
              if (err != INV_ALARM) {
                err = INV_EVENT;
              }
              deleteEvent(tmpEvent);
            }
            if (err != INV_ALARM && err != INV_EVENT) {
              err = INV_CAL;
            }
            deleteCalendar(tmpCal);
            free(line);
            fclose(fp);
            return err;
          } else if (creatingAlarm) {
            insertBack(tmpAlarm->properties, tmpProp);
          } else if (creatingEvent) {
            insertBack(tmpEvent->properties, tmpProp);
          } else {
            insertBack(tmpCal->properties, tmpProp);
          }
      } else if (strcmp(tmpEvent->creationDateTime.date, "temp") != 0) {
        free(line);
        fclose(fp);
        deleteEvent(tmpEvent);
        deleteCalendar(tmpCal);
        err = INV_DT;
        return err;
      } else {
        char* dt = strtok(line, tok);
        dt = strtok(NULL, tok);
        handleDTStamp(dt, &tmpEvent->creationDateTime);
      }
    } else if (startsWith(line, "DTSTART:")) {
      if (!creatingEvent) {
        Property* tmpProp = createProperty(line);
          if (tmpProp == NULL) {
            if (creatingAlarm) {
              err = INV_ALARM;
              deleteAlarm(tmpAlarm);
            }
            if (creatingEvent) {
              if (err != INV_ALARM) {
                err = INV_EVENT;
              }
              deleteEvent(tmpEvent);
            }
            if (err != INV_ALARM && err != INV_EVENT) {
              err = INV_CAL;
            }
            deleteCalendar(tmpCal);
            free(line);
            fclose(fp);
            return err;
          } else if (creatingAlarm) {
            insertBack(tmpAlarm->properties, tmpProp);
          } else if (creatingEvent) {
            insertBack(tmpEvent->properties, tmpProp);
          } else {
            insertBack(tmpCal->properties, tmpProp);
          }
      } else if (strcmp(tmpEvent->startDateTime.date, "temp") != 0) {
        deleteEvent(tmpEvent);
        deleteCalendar(tmpCal);
        free(line);
        fclose(fp);
        err = INV_DT;
        return err;
      } else {
        char* dt = strtok(line, tok);
        dt = strtok(NULL, tok);
        handleDTStamp(dt, &tmpEvent->startDateTime);
      }
    } else if (startsWith(line, "ACTION:")) {
      if (!creatingAlarm) {
        Property* tmpProp = createProperty(line);
          if (tmpProp == NULL) {
            if (creatingAlarm) {
              err = INV_ALARM;
              deleteAlarm(tmpAlarm);
            }
            if (creatingEvent) {
              if (err != INV_ALARM) {
                err = INV_EVENT;
              }
              deleteEvent(tmpEvent);
            }
            if (err != INV_ALARM && err != INV_EVENT) {
              err = INV_CAL;
            }
            deleteCalendar(tmpCal);
            free(line);
            fclose(fp);
            return err;
          } else if (creatingAlarm) {
            insertBack(tmpAlarm->properties, tmpProp);
          } else if (creatingEvent) {
            insertBack(tmpEvent->properties, tmpProp);
          } else {
            insertBack(tmpCal->properties, tmpProp);
          }
      } else if (strcmp(tmpAlarm->action, "temp") != 0) {
        deleteAlarm(tmpAlarm);
        deleteEvent(tmpEvent);
        deleteCalendar(tmpCal);
        free(line);
        fclose(fp);
        err = INV_ALARM;
        return err;
      } else {
        char* action = strtok(line, tok);
        action = strtok(NULL, "");
        strcpy(tmpAlarm->action, action);
      }
    } else if(startsWith(line, "TRIGGER")){
      if (!creatingAlarm) {
        Property* tmpProp = createProperty(line);
          if (tmpProp == NULL) {
            if (creatingAlarm) {
              err = INV_ALARM;
              deleteAlarm(tmpAlarm);
            }
            if (creatingEvent) {
              if (err != INV_ALARM) {
                err = INV_EVENT;
              }
              deleteEvent(tmpEvent);
            }
            if (err != INV_ALARM && err != INV_EVENT) {
              err = INV_CAL;
            }
            deleteCalendar(tmpCal);
            free(line);
            fclose(fp);
            return err;
          } else if (creatingAlarm) {
            insertBack(tmpAlarm->properties, tmpProp);
          } else if (creatingEvent) {
            insertBack(tmpEvent->properties, tmpProp);
          } else {
            insertBack(tmpCal->properties, tmpProp);
          }
      } else if (tmpAlarm->trigger != NULL) {
        deleteAlarm(tmpAlarm);
        deleteEvent(tmpEvent);
        deleteCalendar(tmpCal);
        free(line);
        fclose(fp);
        err = INV_ALARM;
        return err;
      } else {
        char* trigger = strtok(line, tok);
        trigger = strtok(NULL, "");
        tmpAlarm->trigger = malloc(sizeof(char) * strlen(trigger) + 1);
        strcpy(tmpAlarm->trigger, trigger);
      }
    } else {
      // default extra property...
      if (tmpCal == NULL) {
        free(line);
        fclose(fp);
        err = INV_CAL;
        return err;
      }
      Property* tmpProp = createProperty(line);
          if (tmpProp == NULL) {
            if (creatingAlarm) {
              err = INV_ALARM;
              deleteAlarm(tmpAlarm);
            }
            if (creatingEvent) {
              if (err != INV_ALARM) {
                err = INV_EVENT;
              }
              deleteEvent(tmpEvent);
            }
            if (err != INV_ALARM && err != INV_EVENT) {
              err = INV_CAL;
            }
            deleteCalendar(tmpCal);
            free(line);
            fclose(fp);
            return err;
          } else if (creatingAlarm) {
            insertBack(tmpAlarm->properties, tmpProp);
          } else if (creatingEvent) {
            insertBack(tmpEvent->properties, tmpProp);
          } else {
            insertBack(tmpCal->properties, tmpProp);
          }
    }
    free(line);
  }
  fclose(fp);
  if (cur != EOF) {
    err = INV_FILE;
    deleteCalendar(tmpCal);
    if (creatingAlarm) {
      deleteAlarm(tmpAlarm);
    }
    if (creatingEvent) {
      deleteEvent(tmpEvent);
    }
  } else if (creatingAlarm) {
    deleteAlarm(tmpAlarm);
    deleteEvent(tmpEvent);
    deleteCalendar(tmpCal);
    err = INV_ALARM;
  } else if (creatingEvent) {
    deleteEvent(tmpEvent);
    deleteCalendar(tmpCal);
    err = INV_EVENT;
  } else if (!endCal) {
    deleteCalendar(tmpCal);
    err= INV_CAL;
  } else {
    err = validCal(tmpCal);
    if (err != OK) {
      deleteCalendar(tmpCal);
    } else {
      *obj = tmpCal;
    }
  }
  return err;
}

void deleteCalendar(Calendar* obj) {
  if (obj == NULL) {
    return;
  }
  if (obj->properties!=NULL) {
    freeList(obj->properties);
  }
  if (obj->events != NULL) {
    freeList(obj->events);
  }
  free(obj);
}

char* printCalendar(const Calendar* obj) {
  if (obj == NULL) {
    return NULL;
  }
  int len = 100;
  char* evtStr = toString(obj->events);
  int evtLen = strlen(evtStr);
  len+=evtLen;
  char* propStr = toString(obj->properties);
  int propLen = strlen(propStr);
  len+=propLen;

  char* str = malloc(sizeof(char) * len);
  snprintf(str, len, "Version: %.1lf\nProdID: %s\nEvents: %s\nProperties: %s", obj->version, obj->prodID, evtStr, propStr);
  free(evtStr);
  free(propStr);

  return str;
}

char* printError(ICalErrorCode err) {
  char* str;
  if (err == OK) {
    str = malloc(sizeof(char) * 4);
    strcpy(str, "OK\n");
  } else if (err == INV_CAL) {
    str = malloc(sizeof(char) * 20);
    strcpy(str, "Invalid Calendar\n");
  } else if (err == INV_VER) {
    str = malloc(sizeof(char) * 20);
    strcpy(str, "Invalid Version\n");
  } else if (err == DUP_VER) {
    str = malloc(sizeof(char) * 21);
    strcpy(str, "Duplicate Version\n");
  } else if (err == INV_PRODID) {
    str = malloc(sizeof(char) * 22);
    strcpy(str, "Invalid Product ID\n");
  } else if (err == DUP_PRODID) {
    str = malloc(sizeof(char) * 24);
    strcpy(str, "Duplicate Product ID\n");
  } else if (err == INV_EVENT) {
    str = malloc(sizeof(char) * 20);
    strcpy(str, "Invalid Event\n");
  } else if (err == INV_DT) {
    str = malloc(sizeof(char) * 24);
    strcpy(str, "Invalid Date Time\n");
  } else if (err == INV_ALARM) {
    str = malloc(sizeof(char) * 18);
    strcpy(str, "Invalid Alarm\n");
  } else if (err == INV_FILE){
    str = malloc(sizeof(char) * 14);
    strcpy(str, "Invalid File\n");
  } else {
    str = malloc(sizeof(char) * 15);
    strcpy(str, "Other Error\n");
  }
  return str;
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
  int createLen = strlen(createTime);
  char* startTime = printDate(&evt->startDateTime);
  int startLen = strlen(startTime);
  char* propStr = toString(evt->properties);
  int propLen = strlen(propStr);
  char* alarmStr = toString(evt->alarms);
  int alarmLen = strlen(alarmStr);

  int len = createLen+startLen+propLen+alarmLen+100;
  char* str = malloc(sizeof(char) * len);

  snprintf(str, len, "UID: %s\nCreated: %s\nStart: %s\nProperties: %s\nAlarms: %s", evt->UID, createTime, startTime, propStr, alarmStr);

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
  int len = strlen(propStr) + strlen(alarm->action) +strlen(alarm->trigger) +50;

  char* str = malloc(sizeof(char) * len);

  snprintf(str, len, "Action: %s\nTrigger: %s\nProperties: %s", alarm->action, alarm->trigger, propStr);

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
  int len = strlen(prop->propName) + strlen(prop->propDescr) + 4;
  char* str = malloc(sizeof(char) * len);

  snprintf(str, len, "%s: %s\n", prop->propName, prop->propDescr);

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
  strcpy(print, (*dt).date);
  strcat(print, " ");
  strcat(print, (*dt).time);

  if ((*dt).UTC) {
    strcat(print, " UTC");
  } else {
    strcat(print, "");
  }

  return print;
}
