#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "parse_csv.h"

#define INITIAL_ROWS 100
#define GROWTH_FACTOR 1.5
#define BUFFER_SIZE 10

void new_value (void *p, int row, int col, double value)
{
  // handle resizing here
  printf ("mat(%i,%i) = %f\n", row, col, value);
}

void new_comment (void *p, const char* c)
{
  printf ("new_comment '%s'\n", c);
}

int main ()
{
  int current_row_idx = 0;
  int current_col_idx = 0;
  char in_comment = 0;

  char *tail = 0;
  char *buf = tail = (char *) malloc (BUFFER_SIZE);

  int bytes_read = 0;

  while ((bytes_read = read (STDIN_FILENO, tail, BUFFER_SIZE - (tail - buf) - 1)) > 0)
    {
      tail = tail + bytes_read;
      *tail = 0;

      printf ("bytes_read = %i, tail at %li\n", bytes_read, tail-buf);
      parse_csv (buf, &tail, &in_comment, &current_row_idx, &current_col_idx, 0, &new_value, &new_comment);
    }

  free (buf);
  return 0;
}
