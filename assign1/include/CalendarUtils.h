#ifndef CALENDARUTILS_H
#define CALENDARUTILS_H

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int readLine(FILE* fp);
char* fixLine(char* line);
bool startsWith(char* str, char* search);
bool endsWith(char* str, char* search);

#endif
