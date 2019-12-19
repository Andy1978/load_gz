#ifndef _PARSE_CSV_H_
#define _PARSE_CSV_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

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

  #define DBG_PRE() printf ("%s:%s:%i ", __FILE__, __FUNCTION__, __LINE__);

  #define DBG_STR(s) DBG_PRE(); printf ("%s\n",s);
  #define DBG_STR_VAL(s) DBG_PRE(); printf ("%s = '%s'\n", #s, s);
  #define DBG_INT_VAL(s) DBG_PRE(); printf ("%s = %i\n", #s, s);
  #define DBG_DOUBLE_VAL(s) DBG_PRE(); printf ("%s = %f\n", #s, s);

#else //No DEBUG defined

  #define DBG_PRE()
  #define DBG_STR(s)
  #define DBG_STR_VAL(s)
  #define DBG_INT_VAL(s)
  #define DBG_DOUBLE_VAL(s)

#endif

typedef void(*cb_new_value)(void*, int, int, double);
typedef void(*cb_new_comment)(void*, const char* c);

char isEOL (char c);
void parse_csv (char *buf,
                char** tail,
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
