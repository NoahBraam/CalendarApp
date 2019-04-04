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
        if (version == NULL) {
          err = INV_VER;
          deleteCalendar(tmpCal);
          return err;
        }
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
        if (prodID == NULL) {
          err = INV_PRODID;
          deleteCalendar(tmpCal);
          return err;
        }
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
        ICalErrorCode tempErr = handleDTStamp(dt, &tmpEvent->creationDateTime);
        if (tempErr != OK) {
          err = tempErr;
          fclose(fp);
          free(line);
          deleteEvent(tmpEvent);
          deleteCalendar(tmpCal);
          return err;
        }
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
        ICalErrorCode tempErr = handleDTStamp(dt, &tmpEvent->startDateTime);
        if (tempErr != OK) {
          err = tempErr;
          fclose(fp);
          free(line);
          deleteEvent(tmpEvent);
          deleteCalendar(tmpCal);
          return err;
        }
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
  if (!endCal) {
    err = INV_CAL;
    if (creatingAlarm) {
      deleteAlarm(tmpAlarm);
    }
    if (creatingEvent) {
      deleteEvent(tmpEvent);
    }
    deleteCalendar(tmpCal);
    tmpCal = NULL;
    return err;
  }
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
    strcpy(str, "OK");
  } else if (err == INV_CAL) {
    str = malloc(sizeof(char) * 20);
    strcpy(str, "Invalid Calendar");
  } else if (err == INV_VER) {
    str = malloc(sizeof(char) * 20);
    strcpy(str, "Invalid Version");
  } else if (err == DUP_VER) {
    str = malloc(sizeof(char) * 21);
    strcpy(str, "Duplicate Version");
  } else if (err == INV_PRODID) {
    str = malloc(sizeof(char) * 22);
    strcpy(str, "Invalid Product ID");
  } else if (err == DUP_PRODID) {
    str = malloc(sizeof(char) * 24);
    strcpy(str, "Duplicate Product ID");
  } else if (err == INV_EVENT) {
    str = malloc(sizeof(char) * 20);
    strcpy(str, "Invalid Event");
  } else if (err == INV_DT) {
    str = malloc(sizeof(char) * 24);
    strcpy(str, "Invalid Date Time");
  } else if (err == INV_ALARM) {
    str = malloc(sizeof(char) * 18);
    strcpy(str, "Invalid Alarm");
  } else if (err == INV_FILE){
    str = malloc(sizeof(char) * 14);
    strcpy(str, "Invalid File");
  } else {
    str = malloc(sizeof(char) * 15);
    strcpy(str, "Other Error");
  }
  return str;
}

ICalErrorCode writeCalendar(char* fileName, const Calendar* obj) {
  ICalErrorCode err;
  err = OK;

  // Filename is NULL or too short
  if (fileName == NULL || strlen(fileName) < 5) {
    err = WRITE_ERROR;
    return err;
  }

  // Wrong extention
  if(strcmp(fileName + strlen(fileName) - 4, ".ics")) {
    err = WRITE_ERROR;
    return err;
  }

  // Invalid object
  if (obj == NULL) {
    err = WRITE_ERROR;
    return err;
  }

  FILE* fp = fopen(fileName, "w+");

  // Can't open file
  if (fp == NULL) {
    err = WRITE_ERROR;
    return err;
  }
  fprintf(fp, "BEGIN:VCALENDAR\r\n");
  fprintf(fp, "VERSION:%.1lf\r\n", obj->version);
  fprintf(fp, "PRODID:%s\r\n", obj->prodID);
  // Loop for events
  ListIterator evtIter = createIterator(obj->events);
  void* tmpObj;
  Event* curEvt;
   while((tmpObj = nextElement(&evtIter) ) != NULL) {
    curEvt = (Event*)tmpObj;
    fprintf(fp, "BEGIN:VEVENT\r\n");
    fprintf(fp, "UID:%s\r\n", curEvt->UID);
    fprintf(fp, "DTSTAMP:%sT%s", curEvt->creationDateTime.date, curEvt->creationDateTime.time);
    if (curEvt->creationDateTime.UTC) {
      fprintf(fp,"Z");
    }
    fprintf(fp,"\r\n");
    fprintf(fp, "DTSTART:%sT%s", curEvt->startDateTime.date, curEvt->startDateTime.time);
    if (curEvt->startDateTime.UTC) {
      fprintf(fp,"Z");
    }
    fprintf(fp,"\r\n");
    writeProps(fp, curEvt->properties);
    writeAlarms(fp, curEvt->alarms);
    fprintf(fp, "END:VEVENT\r\n");
  }
  writeProps(fp, obj->properties);
  fprintf(fp, "END:VCALENDAR\r\n");
  fclose(fp);
  return err;
}

ICalErrorCode validateCalendar(const Calendar* obj) {
  ICalErrorCode err = OK;

  // Make sure obj exists
  if (obj == NULL) {
    err = INV_CAL;
    return err;
  }

  // Basic Calendar Validation
  if (obj->events == NULL || obj->properties == NULL) {
    err = INV_CAL;
    return err;
  }
  if (strlen(obj->prodID) > 1000 || strcmp(obj->prodID, "") == 0) {
    err = INV_CAL;
    return err;
  }
  if (getLength(obj->events) < 1) {
    err = INV_CAL;
    return err;
  }
  // Calendar property validation...
  if (!validCalProperties(obj->properties)) {
    err = INV_CAL;
    return err;
  }

  void* tmpObj;
  Event* tmpEvent;
  ListIterator evtIter = createIterator(obj->events);
  while ((tmpObj = nextElement(&evtIter)) != NULL) {
    tmpEvent = (Event*)tmpObj;
    if (strlen(tmpEvent->UID) > 1000 || strcmp(tmpEvent->UID, "") == 0) {
      err = INV_EVENT;
      return err;
    }
    if (strlen(tmpEvent->startDateTime.date) < 8 || strlen(tmpEvent->creationDateTime.date) < 8 || strlen(tmpEvent->startDateTime.time) < 6 || strlen(tmpEvent->creationDateTime.time) < 6) {
      return INV_EVENT;
    }
    if (tmpEvent->properties == NULL) {
      err = INV_EVENT;
      return err;
    } else {
      if (!validEventProperties(tmpEvent->properties)) {
        err = INV_EVENT;
        return err;
      }
    }
    if (tmpEvent->alarms == NULL) {
      err = INV_EVENT;
      return err;
    } else {
      Alarm* tmpAlarm;
      ListIterator almIter = createIterator(tmpEvent->alarms);
      // Alarm verification
      while ((tmpObj = nextElement(&almIter)) != NULL) {
        tmpAlarm = (Alarm*)tmpObj;
        if (strlen(tmpAlarm->action) > 200 || strcmp(tmpAlarm->action, "") == 0) {
          err = INV_ALARM;
          return err;
        }
        if (tmpAlarm->trigger == NULL) {
          err = INV_ALARM;
          return err;
        } else if (strcmp(tmpAlarm->trigger, "") == 0) {
          err = INV_ALARM;
          return err;
        }
        if (tmpAlarm->properties == NULL) {
          err = INV_ALARM;
          return err;
        } else {
          if (!validAlarmProperties(tmpAlarm->properties)) {
            err = INV_ALARM;
            return err;
          }
        }
      }
    }
  }
  return OK;
}

void addEvent(Calendar* cal, Event* toBeAdded) {
  if (cal == NULL || toBeAdded == NULL) {
    return;
  }
  if (cal->events == NULL) {
    return;
  }
  insertBack(cal->events, toBeAdded);
}
//============ JSON Functions ============//

char* dtToJSON(DateTime prop) {
  int len = 51;
  char* dtString = malloc(sizeof(char) * len);

  if (prop.UTC) {
    snprintf(dtString, len, "{\"date\":\"%s\",\"time\":\"%s\",\"isUTC\":true}", prop.date, prop.time);
  } else {
    snprintf(dtString, len, "{\"date\":\"%s\",\"time\":\"%s\",\"isUTC\":false}", prop.date, prop.time);
  }
  return dtString;
}

char* eventToJSON(const Event* event) {
  char* evtString;
  if (event == NULL) {
    evtString = malloc(sizeof(char) * 3);
    strcpy(evtString, "{}");
  } else {
    int numProps = 3 + getLength(event->properties);
    int numAlarms = getLength(event->alarms);
    char* dtString = dtToJSON(event->startDateTime);
    char* sumStr, *locStr, *orgStr;
    Property* tempProp = malloc(sizeof(Property));
    strcpy(tempProp->propName, "SUMMARY");
    void* tempSum = NULL;
    void* tempLoc = NULL;
    void* tempOrg = NULL;
    if (numProps > 3) {
      tempSum = findElement(event->properties, &comparePropertiesByName, tempProp);
      strcpy(tempProp->propName, "LOCATION");
      tempLoc = findElement(event->properties, &comparePropertiesByName, tempProp);
      strcpy(tempProp->propName, "ORGANIZER");
      tempOrg = findElement(event->properties, &comparePropertiesByName, tempProp);
    }
    free(tempProp);
    tempProp = NULL;
    // if (tempSum != NULL) {
    //   tempProp = (Property*)tempSum;
    // }
    if (tempSum != NULL) {
      tempProp = (Property*)tempSum;
      sumStr = malloc(sizeof(char) * strlen(tempProp->propDescr) + 1);
      strcpy(sumStr, tempProp->propDescr);
    } else {
      sumStr = malloc(sizeof(char) * 3);
      strcpy(sumStr, "");
    }

    if (tempLoc != NULL) {
      tempProp = (Property*)tempLoc;
      locStr = malloc(sizeof(char) * strlen(tempProp->propDescr) + 1);
      strcpy(locStr, tempProp->propDescr);
    } else {
      locStr = malloc(sizeof(char) * 3);
      strcpy(locStr, "");
    }

    if (tempOrg != NULL) {
      tempProp = (Property*)tempOrg;
      orgStr = malloc(sizeof(char) * strlen(tempProp->propDescr) + 1);
      strcpy(orgStr, tempProp->propDescr);
    } else {
      orgStr = malloc(sizeof(char) * 3);
      strcpy(orgStr, "");
    }

    
    int len = 11 + strlen(dtString) + 100 + strlen(sumStr) + strlen(locStr) + strlen(orgStr);
    evtString = malloc(sizeof(char) * len);
    snprintf(evtString, len, "{\"startDT\":%s,\"numProps\":%d,\"numAlarms\":%d,\"summary\":\"%s\", \"location\":\"%s\",\"organizer\":\"%s\"}", dtString, numProps, numAlarms, sumStr, locStr, orgStr);
    free(dtString);
    free(sumStr);
    free(locStr);
    free(orgStr);
  }
  return evtString;
}

char* eventListToJSON(const List* eventList) {
  char* eventListString;
  if (eventList == NULL) {
    eventListString = malloc(sizeof(char) * 3);
    strcpy(eventListString, "[]");
  } else if (getLength((List*)eventList) == 0) {
    eventListString = malloc(sizeof(char) * 3);
    strcpy(eventListString, "[]");
  } else {
    int totalLen = 3;
    eventListString = malloc(sizeof(char) * totalLen);
    strcpy(eventListString, "[");
    ListIterator iter = createIterator((List*)eventList);
    void* tmpObj;
    Event* tmpEvent;
    while((tmpObj = nextElement(&iter)) != NULL) {
      tmpEvent = (Event*)tmpObj;
      char* evtString = eventToJSON(tmpEvent);
      totalLen+=(strlen(evtString) + 1);
      eventListString = realloc(eventListString, sizeof(char) * totalLen);
      if (strcmp(eventListString, "[") != 0) {
        strcat(eventListString, ",");
      }
      strcat(eventListString, evtString);
      free(evtString);
    }
    strcat(eventListString, "]");
  }
  return eventListString;
}

char* calendarToJSON(const Calendar* cal) {
  char* calJSON;
  if (cal == NULL) {
    calJSON = malloc(sizeof(char) * 3);
    strcpy(calJSON, "{}");
  } else {
    int len = 55 + strlen(cal->prodID);
    calJSON = malloc(sizeof(char) * len);
    snprintf(calJSON, len, "{\"version\":%.0lf,\"prodID\":\"%s\",\"numProps\":%d,\"numEvents\":%d}", cal->version, cal->prodID, 2+getLength(cal->properties), getLength(cal->events));
  }
  return calJSON;
}

Calendar* JSONtoCalendar(const char* str) {
  if (str == NULL) {
    return NULL;
  }
  int len = strlen((char*)str);
  char* tmpStr = malloc(sizeof(char) * len +1);
  strcpy(tmpStr, str);

  if (!startsWith(tmpStr, "{\"version\":") || !endsWith(tmpStr, "\"}")) {
    free(tmpStr);
    return NULL;
  }
  char* token = strtok(tmpStr, ":");
  token = strtok(NULL, ",");
  float version = atof(token);
  token = strtok(NULL, ":");
  if (strcmp(token, "\"prodID\"") != 0) {
    free(tmpStr);
    return NULL;
  }
  token = strtok(NULL, "\"");

  Calendar* cal = initCal(&printEvent, &deleteEvent, &compareEvents, &printProperty, &deleteProperty, &compareProperties);
  cal->version = version;
  strcpy(cal->prodID, token);

  free(tmpStr);
  return cal;
}

Event* JSONtoEvent(const char* str) {
  if (str == NULL) {
    return NULL;
  }
  int len = strlen((char*)str);
  char* tmpStr = malloc(sizeof(char) * len +1);
  strcpy(tmpStr, str);

  if (!startsWith(tmpStr, "{\"UID\":") || !endsWith(tmpStr, "\"}")) {
    free(tmpStr);
    return NULL;
  }
  char* token = strtok(tmpStr, ":");
  token = strtok(NULL, "\"");
  if (token == NULL) {
    free(tmpStr);
    return NULL;
  }
  Event* evt = malloc(sizeof(Event));
  evt->properties = initializeList(&printProperty, &deleteProperty, &compareProperties);
  evt->alarms = initializeList(&printAlarm, &deleteAlarm, &compareAlarms);
  strcpy(evt->UID, token);

  free(tmpStr);

  return evt;
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
