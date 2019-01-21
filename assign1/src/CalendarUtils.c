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
  return evt;
}

Property* createProperty(char* line) {
  Property* prop = malloc(sizeof(Property) + sizeof(char) * strlen(line));
  strcpy(prop->propName, strtok(line, ":"));
  strcpy(prop->propDescr, strtok(NULL, ":"));

  return prop;
}

// ======== String Helper Functs ======== //
int readLine(FILE* fp) {
  char cur;
  // Read a line
  while ((cur = fgetc(fp)) != '\n');
  int endLine = ftell(fp);
  // Handle line folding
  if ((cur = fgetc(fp)) == ' ' || cur == '\t') {
    endLine = readLine(fp);
  }

  return endLine;
}

char* fixLine(char* line) {
  int len = strlen(line);

  for (int i = 0; i<len; i++) {
    // Deals with folding... sorta
    if (line[i] == '\n' && i != len-1) {
      line[i] = ' ';
      line[i+1] = ' ';
    }
  }
  return line;
}

bool startsWith(char* str, char* search) {
  bool starts = true;
  int len = strlen(search);

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
  for (int i = 0; i<len; i++) {
    if (str[strlen(str)-(len-i)] != search[i]) {
      ends = false;
    }
  }
  return ends;
}
