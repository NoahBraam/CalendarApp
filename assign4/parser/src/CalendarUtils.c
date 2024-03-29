/**
 * Noah Braam
 * 0960202
 */
#include "CalendarUtils.h"

Calendar* initCal(char* (*printFunction1)(void* toBePrinted),void (*deleteFunction1)(void* toBeDeleted),int (*compareFunction1)(const void* first,const void* second), char* (*printFunction2)(void* toBePrinted),void (*deleteFunction2)(void* toBeDeleted),int (*compareFunction2)(const void* first,const void* second)) {
  Calendar* cal = malloc(sizeof(Calendar));
  cal->events = initializeList(printFunction1, deleteFunction1, compareFunction1);
  cal->properties = initializeList(printFunction2, deleteFunction2, compareFunction2);
  cal->version = 0.0;
  strcpy(cal->prodID, "temp");
  return cal;
}

Event* initEvent(char* (*printFunction1)(void* toBePrinted),void (*deleteFunction1)(void* toBeDeleted),int (*compareFunction1)(const void* first,const void* second), char* (*printFunction2)(void* toBePrinted),void (*deleteFunction2)(void* toBeDeleted),int (*compareFunction2)(const void* first,const void* second)) {
  Event* evt = malloc(sizeof(Event));
  evt->properties = initializeList(printFunction1, deleteFunction1, compareFunction1);
  evt->alarms = initializeList(printFunction2, deleteFunction2, compareFunction2);
  strcpy(evt->UID, "temp");
  strcpy(evt->creationDateTime.date, "temp");
  strcpy(evt->startDateTime.date, "temp");
  return evt;
}

Alarm* initAlarm(char* (*printFunction1)(void* toBePrinted),void (*deleteFunction1)(void* toBeDeleted),int (*compareFunction1)(const void* first,const void* second)) {
  Alarm* alarm = malloc(sizeof(Alarm));
  strcpy(alarm->action, "temp");
  alarm->trigger = NULL;
  alarm->properties = initializeList(printFunction1, deleteFunction1, compareFunction1);
  return alarm;
}

Property* createProperty(char* line) {
  Property* prop = malloc(sizeof(Property) + sizeof(char) * strlen(line));
  char tempName[200];
  int i = 0;
  while(1==1) {
    if (line[i] == ';' || line[i] == ':') {
      break;
    }
    tempName[i] = line[i];
    i++;
    if (i == strlen(line)) {
      free(prop);
      return NULL;
    }
  }
  if (i == 0) {
    free(prop);
    return NULL;
  }
  tempName[i] = '\0';
  int len = strlen(line) - i;
  if (len == 0) {
    deleteProperty(prop);
    return NULL;
  }
  char* tempDesc = malloc(sizeof(char) * (len + 1));
  i++;
  for(int j=0; j<len; j++) {
    tempDesc[j] = line[i+j];
  }
  tempDesc[len] = '\0';
  if (strlen(tempDesc) == 0) {
    free(tempDesc);
    deleteProperty(prop);
    return NULL;
  }
  strcpy(prop->propName, tempName);
  strcpy(prop->propDescr, tempDesc);

  free(tempDesc);

  return prop;
}

ICalErrorCode handleDTStamp(char* dt, DateTime* toChange) {
  if (endsWith(dt, "Z")) {
    (*toChange).UTC = true;
  } else {
    (*toChange).UTC = false;
  }
  char* token  = strtok(dt, "T");
  strcpy((*toChange).date, token);
  if (strlen((*toChange).date) != 8) {
    return INV_DT;
  }
  token = strtok(NULL,"Z");
  if (token == NULL) {
    return INV_DT;
  }
  strcpy((*toChange).time, token);

  return OK;
}

bool validEvent(Event* evt) {
  if (strcmp(evt->creationDateTime.date, "temp") == 0 || strcmp(evt->startDateTime.date, "temp") == 0) {
    return false;
  }
  if (strcmp(evt->UID, "temp") == 0) {
    return false;
  }
  return true;
}

bool validAlarm(Alarm* alarm) {
  if (alarm == NULL) {
    return false;
  }
  if (strcmp(alarm->action, "temp") == 0 || alarm->trigger == NULL) {
    return false;
  }
  return true;
}

ICalErrorCode validCal(Calendar* cal) {
  if (cal->version == 0.0) {
    return INV_CAL;
  }
  if (strcmp(cal->prodID, "temp") == 0) {
    return INV_CAL;
  }
  if (getLength(cal->events) < 1) {
    return INV_CAL;
  }
  return OK;
}

void writeProps(FILE* fp, List* props) {
  if (props->length < 1) {
    return;
  }
  ListIterator propsIter = createIterator(props);
  void* tmpObj;
  Property* tmpProp;
  while((tmpObj = nextElement(&propsIter)) != NULL) {
    tmpProp = (Property*)tmpObj;
    fprintf(fp, "%s:%s\r\n", tmpProp->propName, tmpProp->propDescr);
  }
}

void writeAlarms(FILE* fp, List* alarms) {
  if (alarms->length < 1) {
    return;
  }
  ListIterator alarmsIter = createIterator(alarms);
  void* tmpObj;
  Alarm* tmpAlarm;
  while((tmpObj = nextElement(&alarmsIter)) != NULL) {
    tmpAlarm = (Alarm*)tmpObj;
    fprintf(fp, "BEGIN:VALARM\r\n");
    fprintf(fp, "TRIGGER;%s\r\n", tmpAlarm->trigger);
    fprintf(fp, "ACTION:%s\r\n", tmpAlarm->action);
    writeProps(fp, tmpAlarm->properties);
    fprintf(fp, "END:VALARM\r\n");
  }
}

bool comparePropertiesByName(const void* first, const void* second) {
  Property* prop1 = (Property*)first;
  Property* prop2 = (Property*)second;

  if (strcmp(prop1->propName, prop2->propName) == 0) {
    return true;
  }
  return false;
}

bool allPropsValid(List* properties) {
  ListIterator iter = createIterator(properties);
  void* tmpObj;
  Property* prop;
  while ((tmpObj = nextElement(&iter)) != NULL) {
    prop = (Property*)tmpObj;
    if (prop->propDescr == NULL) {
      return false;
    }
    if (strcmp(prop->propDescr, "") == 0) {
      return false;
    }
    if (strlen(prop->propName) > 200) {
      return false;
    }
  }
  return true;
}

int numPropertiesWithName(List* properties, char* name) {
  int numMatches = 0;
  ListIterator iter = createIterator(properties);
  void* tmpObj;
  Property* prop;
  while ((tmpObj = nextElement(&iter)) != NULL) {
    prop = (Property*)tmpObj;
    if (strcmp(prop->propName, name) == 0) {
      numMatches++;
    }
  }
  return numMatches;
}

bool onlyValidAlarmPropNames(List* properties) {
  ListIterator iter = createIterator(properties);
  void* tmpObj;
  Property* prop;
  while ((tmpObj = nextElement(&iter)) != NULL) {
    prop = (Property*)tmpObj;
    if (strcmp(prop->propName, "DURATION") != 0 && strcmp(prop->propName, "REPEAT") != 0 && strcmp(prop->propName, "ATTACH") != 0) {
      return false;
    }
  }
  return true;
}

bool validAlarmProperties(List* properties) {
  if (!allPropsValid(properties)) {
    return false;
  }
  int numDur = numPropertiesWithName(properties, "DURATION");
  int numRepeat = numPropertiesWithName(properties, "REPEAT");
  if (numDur != numRepeat || numDur > 1) {
    return false;
  } 
  int numAttach = numPropertiesWithName(properties, "ATTACH");
  if (numAttach > 1) {
    return false;
  }
  int propLen = getLength(properties);
  if (propLen > 3) {
    return false;
  }
  return onlyValidAlarmPropNames(properties);
}

bool onlyValidEventPropNames(List* properties) {
  ListIterator iter = createIterator(properties);
  void* tmpObj;
  Property* prop;
  while ((tmpObj = nextElement(&iter)) != NULL) {
    prop = (Property*)tmpObj;
    if (strcmp(prop->propName, "CLASS") != 0 && strcmp(prop->propName, "CREATED") != 0 && strcmp(prop->propName, "DESCRIPTION") != 0 && strcmp(prop->propName, "GEO") != 0 && strcmp(prop->propName, "LAST-MODIFIED") != 0 && strcmp(prop->propName, "LOCATION") != 0 && strcmp(prop->propName, "ORGANIZER") != 0 && strcmp(prop->propName, "PRIORITY") != 0 && strcmp(prop->propName, "SEQUENCE") != 0 && strcmp(prop->propName, "STATUS") != 0 &&
        strcmp(prop->propName, "SUMMARY") != 0 && strcmp(prop->propName, "TRANSP") != 0 && strcmp(prop->propName, "URL") != 0 && strcmp(prop->propName, "RECURRENCE-ID") != 0 && strcmp(prop->propName, "RRULE") != 0 && strcmp(prop->propName, "DTEND") != 0 && strcmp(prop->propName, "DURATION") != 0 && strcmp(prop->propName, "ATTACH") != 0 && strcmp(prop->propName, "ATTENDEE") != 0 && strcmp(prop->propName, "CATEGORIES") != 0 &&
        strcmp(prop->propName, "COMMENT") != 0 && strcmp(prop->propName, "CONTACT") != 0 && strcmp(prop->propName, "EXDATE") != 0 && strcmp(prop->propName, "RSTATUS") != 0 && strcmp(prop->propName, "RELATED") != 0 && strcmp(prop->propName, "RESOURCES") != 0 && strcmp(prop->propName, "RDATE") != 0) {
          return false;
        }
  }
  return true;
}

bool validEventProperties(List* properties) {
  if (!allPropsValid(properties)) {
    return false;
  }

  int numDur = numPropertiesWithName(properties, "DURATION");
  int numDtEnd = numPropertiesWithName(properties, "DTEND");
  // num dtend * num duration must = 0.
  if ((numDur * numDtEnd) != 0) {
    return false;
  }

  // These are the properties that can only be there once
  char *strs[14] = {"CLASS", "CREATED", "DESCRIPTION", "GEO", "LAST-MODIFIED", "LOCATION", "ORGANIZER", "PRIORITY", "SEQUENCE", "STATUS", "SUMMARY", "TRANSP", "URL", "RECURRENCE-ID"};
  for (int i = 0; i<14; i++) {
    int numTimes = numPropertiesWithName(properties, strs[i]);
    if (numTimes > 1) {
      return false;
    }
  }
  return onlyValidEventPropNames(properties);
}

bool onlyValidCalPropNames(List* properties) {
  ListIterator iter = createIterator(properties);
  void* tmpObj;
  Property* prop;
  while ((tmpObj = nextElement(&iter)) != NULL) {
    prop = (Property*)tmpObj;
    if (strcmp(prop->propName, "CALSCALE") != 0 && strcmp(prop->propName, "METHOD") != 0) {
      return false;
    }
  }
  return true;
}

bool validCalProperties(List* properties) {
  if (!allPropsValid(properties)) {
    return false;
  }
  if (getLength(properties) > 2) {
    return false;
  }
  // These are the properties that can only be there once
  char *strs[2] = {"CALSCALE", "METHOD"};
  for (int i = 0; i<2; i++) {
    int numTimes = numPropertiesWithName(properties, strs[i]);
    if (numTimes > 1) {
      return false;
    }
  }
  return onlyValidCalPropNames(properties);
}

// ======== String Helper Functs ======== //
char* readLine(FILE* fp) {
  int lineStart, lineEnd;
  lineStart = ftell(fp) -1;
  char cur, prev;
  // Read a line
  while ((cur = fgetc(fp)) != '\n') {
    if (cur == EOF) {
      return NULL;
    }
    prev = cur;
  }
  lineEnd = ftell(fp);
  int len = lineEnd - lineStart-1;
  char* line = malloc(sizeof(char) * len);

  fseek(fp, lineStart, SEEK_SET);
  fgets(line, len, fp);

  fseek(fp, lineEnd, SEEK_SET);
  char* newLine;
  // Handle line folding
  if ((cur = fgetc(fp)) == ' ' || cur == '\t' || prev != '\r') {
    fseek(fp, ftell(fp) + 1, SEEK_SET);
    newLine = readLine(fp);
    if (newLine == NULL) {
      free(line);
      return NULL;
    }
    char* tmpLine = line;
    line = malloc(sizeof(char) * (len + strlen(newLine)));
    snprintf(line, len + strlen(newLine), "%s%s", tmpLine, newLine);
    free(tmpLine);
    free(newLine);
  } else {
      fseek(fp, lineEnd, SEEK_SET);
  }

  return line;
}

bool startsWith(char* str, char* search) {
  bool starts = true;
  int len = strlen(search);
  if (strlen(str) < strlen(search))
    return false;
  for (int i = 0; i<len; i++) {
    if (str[i] != search[i]) {
      starts = false;
    }
  }

  return starts;
}

bool endsWith(char* str, char* search) {
  bool ends = true;
  int len = strlen(search);
  if (strlen(str) < strlen(search))
    return false;
  for (int i = 0; i<len; i++) {
    if (str[strlen(str)-(len-i)] != search[i]) {
      ends = false;
    }
  }
  return ends;
}

char* parseCalReturnJSON(char* file) {
  int len = strlen(file);
  int num = 0;
  Calendar* cal;
  ICalErrorCode err = createCalendar(file, &cal);
  if (err == OK) {
    err = validateCalendar(cal);
    if (err == OK) {
      char* calJSON = calendarToJSON(cal);
      deleteCalendar(cal);
      return calJSON;
    } else {
      num = 2;
    }
  } else {
    num = 1;
  }
  char* errString = printError(err);
  char* finalJSON = malloc(sizeof(char) * 30 + strlen(errString));

  len = strlen(errString) + 30;
  snprintf(finalJSON, len, "{\"err\": \"%s\", \"num\": %d }", errString, num);
  return finalJSON;
}

char* parseCalReturnEvents(char* file) {
  Calendar* cal;
  ICalErrorCode err = createCalendar(file, &cal);
  if (err == OK) {
    err = validateCalendar(cal);
    if (err == OK) {
      char* evtJSON = eventListToJSON(cal->events);
      deleteCalendar(cal);
      return evtJSON;
    }
  }
  char* errString = printError(err);
  char* finalJSON = malloc(sizeof(char) * 30 + strlen(errString));

  int len = strlen(errString) + 30;
  snprintf(finalJSON, len, "{\"err\": \"%s\", \"num\": %d }", errString, 1);
  free(errString);
  return finalJSON;
}

char* addEventToFile(char* filename, char* evtJSON, char* createDT, char* startDT, char* summary) {
  Calendar* cal;
  ICalErrorCode err = createCalendar(filename, &cal);

  Event* newEvt = JSONtoEvent(evtJSON);
  if (newEvt == NULL) {
    return "{\"err\":\"nullEVT\"}";
  }
  DateTime start, create;
  char* tok = strtok(startDT, "T");
  strcpy(start.date, tok);
  tok = strtok(NULL, "Z");
  strcpy(start.time, tok);

  tok = strtok(createDT, "T");
  strcpy(create.date, tok);
  tok = strtok(NULL, "Z");
  strcpy(create.time, tok);

  newEvt->creationDateTime = create;
  newEvt->startDateTime = start;

  if (strcmp(summary, "") != 0) {
    Property* property = malloc(sizeof(Property) + strlen(summary) + 1);
    strcpy(property->propName, "SUMMARY");
    strcpy(property->propDescr, summary);
    insertBack(newEvt->properties, property);
  }
  addEvent(cal, newEvt);
  err = validateCalendar(cal);
  if (err == OK) {
    err = writeCalendar(filename, cal);
    deleteCalendar(cal);
  }

  char* errString = printError(err);
  char* finalJSON = malloc(sizeof(char) * 30 + strlen(errString));

  int len = strlen(errString) + 30;
  snprintf(finalJSON, len, "{\"err\": \"%s\", \"num\": %d }", errString, 1);
  free(errString);
  return finalJSON;
}

char* newCalendarFile(char* filename, char* calJSON, char* evtJSON, char* createDT, char* startDT, char* summary) {
  Calendar* cal = JSONtoCalendar(calJSON);

  Event* newEvt = JSONtoEvent(evtJSON);
  if (newEvt == NULL) {
    return "{\"err\":\"nullEVT\"}";
  }
  DateTime start, create;
  char* tok = strtok(startDT, "T");
  strcpy(start.date, tok);
  tok = strtok(NULL, "Z");
  strcpy(start.time, tok);

  tok = strtok(createDT, "T");
  strcpy(create.date, tok);
  tok = strtok(NULL, "Z");
  strcpy(create.time, tok);

  newEvt->creationDateTime = create;
  newEvt->startDateTime = start;

  if (strcmp(summary, "") != 0) {
    Property* property = malloc(sizeof(Property) + strlen(summary) + 1);
    strcpy(property->propName, "SUMMARY");
    strcpy(property->propDescr, summary);
    insertBack(newEvt->properties, property);
  }
  addEvent(cal, newEvt);
  ICalErrorCode err = validateCalendar(cal);
  if (err == OK) {
    err = writeCalendar(filename, cal);
    deleteCalendar(cal);
  }

  char* errString = printError(err);
  char* finalJSON = malloc(sizeof(char) * 30 + strlen(errString));

  int len = strlen(errString) + 30;
  snprintf(finalJSON, len, "{\"err\": \"%s\", \"num\": %d }", errString, 1);
  free(errString);
  return finalJSON;
}

char* getAlarmListOfEvent(char* filename, int eventNumber) {
  Calendar* cal;
  createCalendar(filename, &cal);

  ListIterator iter = createIterator((List*)cal->events);
  void* tmpObj;
  Event* tmpEvent;
  for (int i = 0; i< eventNumber; i++) {
    tmpObj = nextElement(&iter);
  }
  tmpEvent = (Event*)tmpObj;
  char* almJSON = alarmListToJSON(tmpEvent->alarms);
  deleteCalendar(cal);
  return almJSON;
}

char* getPropertyListOfEvent(char* filename, int eventNumber) {
  Calendar* cal;
  createCalendar(filename, &cal);

  ListIterator iter = createIterator((List*)cal->events);
  void* tmpObj;
  Event* tmpEvent;
  for (int i = 0; i< eventNumber; i++) {
    tmpObj = nextElement(&iter);
  }
  tmpEvent = (Event*)tmpObj;
  char* propJSON = propertyListToJSON(tmpEvent->properties);
  deleteCalendar(cal);
  return propJSON;
}

char* propertyToJSON(const Property* prop) {
  Property* tmpProp = (Property*) prop;
  int len = strlen(tmpProp->propName) + strlen(tmpProp->propDescr) + 45;

  char* propJSON = malloc(sizeof(char) * len);
  snprintf(propJSON, len, "{\"name\":\"%s\",\"descr\":\"%s\"}", tmpProp->propName, tmpProp->propDescr);
  return propJSON;
}

char* propertyListToJSON(const List* propList) {
  char* propListString;
  if (propList == NULL) {
    propListString = malloc(sizeof(char) * 3);
    strcpy(propListString, "[]");
  } else if (getLength((List*)propList) == 0) {
    propListString = malloc(sizeof(char) * 3);
    strcpy(propListString, "[]");
  } else {
    int totalLen = 3;
    propListString = malloc(sizeof(char) * totalLen);
    strcpy(propListString, "[");
    ListIterator iter = createIterator((List*)propList);
    void* tmpObj;
    Property* tmpProp;
    while((tmpObj = nextElement(&iter)) != NULL) {
      tmpProp = (Property*)tmpObj;
      char* evtString = propertyToJSON(tmpProp);
      totalLen+=(strlen(evtString) + 1);
      propListString = realloc(propListString, sizeof(char) * totalLen);
      if (strcmp(propListString, "[") != 0) {
        strcat(propListString, ",");
      }
      strcat(propListString, evtString);
      free(evtString);
    }
    strcat(propListString, "]");
  }
  return propListString;
}

char* alarmToJSON (const Alarm* alm) {
  Alarm* tmpAlm = (Alarm*) alm;
  char* propList = propertyListToJSON(tmpAlm->properties);
  int len = strlen(tmpAlm->action) + strlen(tmpAlm->trigger) +strlen(propList) + 45;
  char* almJSON = malloc(sizeof(char) * len);

  snprintf(almJSON, len, "{\"action\":\"%s\",\"trigger\":\"%s\",\"properties\":%s}", tmpAlm->action, tmpAlm->trigger, propList);
  free(propList);
  return almJSON;
}

char* alarmListToJSON(const List* alarmList) {
  char* alarmListString;
  if (alarmList == NULL) {
    alarmListString = malloc(sizeof(char) * 3);
    strcpy(alarmListString, "[]");
  } else if (getLength((List*)alarmList) == 0) {
    alarmListString = malloc(sizeof(char) * 3);
    strcpy(alarmListString, "[]");
  } else {
    int totalLen = 3;
    alarmListString = malloc(sizeof(char) * totalLen);
    strcpy(alarmListString, "[");
    ListIterator iter = createIterator((List*)alarmList);
    void* tmpObj;
    Alarm* tmpAlarm;
    while((tmpObj = nextElement(&iter)) != NULL) {
      tmpAlarm = (Alarm*)tmpObj;
      char* evtString = alarmToJSON(tmpAlarm);
      totalLen+=(strlen(evtString) + 1);
      alarmListString = realloc(alarmListString, sizeof(char) * totalLen);
      if (strcmp(alarmListString, "[") != 0) {
        strcat(alarmListString, ",");
      }
      strcat(alarmListString, evtString);
      free(evtString);
    }
    strcat(alarmListString, "]");
  }
  return alarmListString;
}