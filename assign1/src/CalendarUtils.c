#include "CalendarUtils.h"

int readLine(FILE* fp) {
  char cur;
  // Read a line
  while ((cur = fgetc(fp)) != '\n');
  int endLine = ftell(fp);
  if ((cur = fgetc(fp)) == ' ' || cur == '\t') {
    endLine = readLine(fp);
  }

  return endLine;
}

char* fixLine(char* line) {
  int len = strlen(line);

  for (int i = 0; i<len; i++) {
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
