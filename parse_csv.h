#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#ifndef _PARSE_CSV_H_
#define _PARSE_CSV_H_

typedef void(*cb_new_value)(void*, int, int, double);
typedef void(*cb_new_comment)(void*, const char* c);

char isEOL (char c);
void parse (char *buf, char** tail, char *in_comment, int *current_row_idx, int *current_col_idx, cb_new_value cb_nv, cb_new_comment cb_nc);

#endif
