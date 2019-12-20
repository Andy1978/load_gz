#ifndef _PARSE_CSV_H_
/*
  parse_csv: parse CSV with numerical data and comments

  Copyright (C) 2019 Andreas Weber

  This software is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; see the file LICENSE.  If not, see
  <https://www.gnu.org/licenses/>.
*/

#define _PARSE_CSV_H_

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*************************     debugging     **************************/

#ifdef DEBUG

  // a = first user defined parameter
  // b = second user defined parameter

  // example:
  // DBG_MSG2 ("foo", 8);
  // writes to stdout:
  // save_json.cc:save_matrix   :113  foo 8

  #define DBG_PRE() printf ("%s:%s:%i ", __FILE__, __FUNCTION__, __LINE__)

  #define DBG_STR(s) DBG_PRE(); printf ("%s\n",s)
  #define DBG_STR_VAL(s) DBG_PRE(); printf ("%s = '%s'\n", #s, s)
  #define DBG_INT_VAL(s) DBG_PRE(); printf ("%s = %i\n", #s, s)
  #define DBG_LINT_VAL(s) DBG_PRE(); printf ("%s = %li\n", #s, s)
  #define DBG_DOUBLE_VAL(s) DBG_PRE(); printf ("%s = %f\n", #s, s)

#else //No DEBUG defined

  #define DBG_PRE()
  #define DBG_STR(s)
  #define DBG_STR_VAL(s)
  #define DBG_INT_VAL(s)
  #define DBG_LINT_VAL(s)
  #define DBG_DOUBLE_VAL(s)

#endif

typedef void(*cb_new_value)(void*, int, int, double);
typedef void(*cb_new_comment)(void*, char append, char complete, const char* c);

char isEOL (char c);
void parse_csv (char *buf,
                char** tail,
                char flush,
                char *in_comment,
                int *current_row_idx,
                int *current_col_idx,
                void* userdata,
                cb_new_value cb_nv,
                cb_new_comment cb_nc);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
