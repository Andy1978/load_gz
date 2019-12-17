#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#ifndef _PARSE_CSV_H_
#define _PARSE_CSV_H_

char isEOL (char c);
void parse (char *buf, char** tail);

void cb_new_value (int row, int col, double value);
void cb_new_comment (const char* c);

#endif
