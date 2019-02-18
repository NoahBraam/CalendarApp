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
