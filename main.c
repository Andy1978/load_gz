#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "parse_csv.h"

#define INITIAL_ROWS 100
#define GROWTH_FACTOR 1.5
#define BUFFER_SIZE 10

void cb_new_value (int row, int col, double value)
{
  printf ("mat(%i,%i) = %f\n", row, col, value);
}

void cb_new_comment (const char* c)
{
  printf ("new_comment '%s'\n", c);
}

int main ()
{
  char *tail = 0;
  char *buf = tail = (char *) malloc (BUFFER_SIZE);

  int bytes_read = 0;

  while ((bytes_read = read (STDIN_FILENO, tail, BUFFER_SIZE - (tail - buf) - 1)) > 0)
    {
      tail = tail + bytes_read;
      *tail = 0;

      printf ("bytes_read = %i, tail at %li\n", bytes_read, tail-buf);
      parse (buf, &tail);
    }

  free (buf);
  return 0;
}
