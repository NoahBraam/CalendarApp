#include <stdio.h>
#include <stdlib.h>
#include "CalendarParser.h"

int main(int argc, char** argv) {
  printf("Hello world!\n");

  Calendar* cal;

  char* err = printError(createCalendar("wikipedia1.ics", &cal));

  printf("%s", err);

}