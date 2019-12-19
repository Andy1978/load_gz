/*
 * helper tool to debug and fuzz the "parse_csv" function
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include "parse_csv.h"

#define INITIAL_ROWS 100
#define GROWTH_FACTOR 1.5
#define BUFFER_SIZE 20

static char last_cb_was_comment = 1;

// callback is called if a new double value is parsed
void new_value (void *p, int row, int col, double value)
{
  (void) p;
  //printf ("mat(%i,%i) = %f\n", row, col, value);

  static int last_row = 0;
  if (row > last_row && !last_cb_was_comment)
    putchar ('\n');

  if (col > 0)
    putchar (' ');

  printf ("%f", value);
  last_row = row;
  last_cb_was_comment = 0;
}

// callback is called if a (portion) of comment is parsed
void new_comment (void *p, char append, char complete, const char* c)
{
  (void) p;
  //printf ("p = %p, append = %i, complete = %i, c = '%s'\n", p, append, complete, c);
#if 1
  if (! append && !last_cb_was_comment)
    putchar ('\n');
  fputs (c, stdout);
  if (complete)
    putchar ('\n');
#endif
  last_cb_was_comment = 1;
}

int main (int argc, char *argv[])
{
  int fd = STDIN_FILENO;
  if (argc > 1)
    {
      fd = open (argv[1], O_RDONLY);
      if (fd < 0)
        {
          fprintf (stderr, "ERROR: Opening input file '%s' failed: %s\n", argv[1], strerror (errno));
          fprintf (stderr, "INFO: %s can read from stdin or file\n", argv[0]);
          return -1;
        }
    }

  int current_row_idx = 0;
  int current_col_idx = 0;
  char in_comment = 0;

  char *tail = 0;
  char *buf = tail = (char *) malloc (BUFFER_SIZE);

  int bytes_read = 0;

  while ((bytes_read = read (fd, tail, BUFFER_SIZE - (tail - buf) - 1)) > 0)
    {
      tail = tail + bytes_read;
      *tail = 0;
#ifdef DEBUG
      printf ("bytes_read = %i, tail at %li\n", bytes_read, tail-buf);
#endif
      parse_csv (buf, &tail, &in_comment, &current_row_idx, &current_col_idx, 0, &new_value, &new_comment);
    }
  putchar ('\n');
  free (buf);
  if (fd != STDIN_FILENO)
    close (fd);
  return 0;
}
