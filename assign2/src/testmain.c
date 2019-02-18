#include <stdio.h>
#include <stdlib.h>
#include "CalendarParser.h"

int main(int argc, char** argv) {
  printf("Hello world!\n");

  Calendar* cal;
  char* err = printError(createCalendar(argv[1], &cal));
  printf("%s\n", err);
  free(err);
  if (cal != NULL) {
    writeCalendar("test.ics", cal);
    char* calS = printCalendar(cal);
    printf("%s\n", calS);
    free(calS);
    deleteCalendar(cal);
  }
  DateTime testDt;
  strcpy(testDt.date, "12345678");
  strcpy(testDt.time, "987654");
  testDt.UTC = false;

  char* string = dtToJSON(testDt);
  printf("%s\n", string);
  free(string);
  testDt.UTC = true;
  string = dtToJSON(testDt);
  printf("%s\n", string);
  free(string);
}
