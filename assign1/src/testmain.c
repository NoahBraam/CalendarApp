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
    char* calS = printCalendar(cal);
    printf("%s\n", calS);
    free(calS);
    deleteCalendar(cal);
  }

}
