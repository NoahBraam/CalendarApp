#include <stdio.h>
#include <stdlib.h>
#include "CalendarParser.h"

int main(int argc, char** argv) {
  printf("Hello world!\n");

  Calendar* cal;
  char* err = printError(createCalendar(argv[1], &cal));
  printf("%s\n", err);
  free(err);
  err = printError(validateCalendar(cal));
  printf("%s\n", err);
  free(err);
  if (cal != NULL) {
    writeCalendar("test.ics", cal);
    char* calS = printCalendar(cal);
    printf("%s\n", calS);
    free(calS);
    char* calJSON = calendarToJSON(cal);
    printf("%s\n", calJSON);
    free(calJSON);
    char* evtJSON = eventListToJSON(cal->events);
    printf("%s\n", evtJSON);
    free(evtJSON);
    deleteCalendar(cal);
  }
  char* jsonCal = malloc(sizeof(char) * 70);
  strcpy(jsonCal, "{\"version\":2,\"prodID\":\"1234567\"}");
  printf("%s\n", jsonCal);
  cal = JSONtoCalendar(jsonCal);
  char* tmp = printCalendar(cal);
  printf("%s\n", tmp);
  deleteCalendar(cal);
  free(tmp);
  free(jsonCal);
}
