/*
 * helper tool to debug and fuzz the "parse_csv" function
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include "parse_csv.h"

static int buffer_size = 500;
static char last_cb_was_comment = 1;

const char *program_name = "parse";
const char *program_version = "0.0.1";
const char *program_bug_address = "<andy@josoansi.de>";

// callback is called if a new double value is parsed
void new_value (void *p, int row, int col, double value)
{
  (void) p;
  //printf ("mat(%i,%i) = %f\n", row, col, value);

  static int last_row = 0;
  if (row > last_row && !last_cb_was_comment)
    putchar ('\n');

  if (col > 0)
    putchar (';');

  printf ("%f", value);
  last_row = row;
  last_cb_was_comment = 0;
}

// callback is called if a (portion) of comment is parsed
// c can be NULL
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

void usage ()
{
  printf ("Usage: %s [OPTION] [FILENAME]\n", program_name);
  fputs ("\
\n\
  If FILENAME is given, parse it. Without FILENAME stdin is used.\n\
\n\
 options:\n\
\n\
 -b, --buffersize   Set buffer size, default is 500\n\
     --help         Give this help list\n\
 -V, --version      Print program version (do not connect)\n\n\
", stdout);
}

static struct option const long_options[] =
{
  {"buffersize", required_argument, NULL, 'b'},
  {"help", no_argument, NULL, 'h'},
  {"version", no_argument, NULL, 'V'},
  {NULL, 0, NULL, 0}
};

void test_strtod (const char* start)
{
  char* end;
  double d = strtod (start, &end);
  printf ("start = '%s', d = %f, end-start = %li, *end = %i = '%c'\n", start, d, end-start, *end, *end);
}

int main (int argc, char *argv[])
{
  //~ test_strtod ("1.23");
  //~ test_strtod (" 2.34 ");
  //~ test_strtod ("  ");
  //~ test_strtod (";3.14");
  //~ test_strtod ("9e");
  //~ test_strtod ("8E");
  //~ test_strtod ("3.");
  //~ test_strtod ("0x4.25p+2");
  //~ test_strtod ("0x0.25");
  //~ return 0;

  /* getopt_long stores the option index here. */
  int option_index;
  while (1)
    {
      option_index = -1;
      int c = getopt_long (argc, argv, "hVb:",
                           long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        break;

      //~ if (option_index >= 0)
      //~ printf ("Processing long option '%s'\n", long_options[option_index].name);
      //~ else if (c > 0)
      //~ printf ("Processing short option '%c'\n", c);

      switch (c)
        {
        case 0:
          /* If this option set a flag, do nothing else now. */
          break;

        case 'h': // help
          usage ();
          return 0;

        case 'V': // version
          printf ("%s version %s, Copyright (c) 2019 Andreas Weber\n",
                  program_name,
                  program_version);
          printf (" built on %s %s. ", __DATE__, __TIME__);
          printf ("Please report problems to %s\n", program_bug_address);
          return 0;

        case 'b':
          buffer_size = atoi (optarg);
          break;

        case '?':
          /* getopt_long already printed an error message. */
          printf ("Use '--help' to see possible options\n");
          break;

        default:
          fprintf (stderr, "Error: This should never happen...\n");
          abort ();
        }
    }

  if (buffer_size < 20)
    {
      fprintf (stderr, "ERROR: buffersize = %i is too small...\n", buffer_size);
      return -1;
    }

  int fd = STDIN_FILENO;
  if ((argc - optind) > 0)
    {
      fd = open (argv[optind], O_RDONLY);
      if (fd < 0)
        {
          fprintf (stderr, "ERROR: Opening input file '%s' failed: %s\n", argv[optind], strerror (errno));
          return -1;
        }
    }

  int current_row_idx = 0;
  int current_col_idx = 0;
  char in_comment = 0;

  char *tail = 0;
  //printf ("buffersize = %i\n", buffer_size);
  char *buf = tail = (char *) malloc (buffer_size);

  int bytes_read = 0;

  do
    {
      bytes_read = read (fd, tail, buffer_size - (tail - buf) - 1);
      tail = tail + bytes_read;
      *tail = 0;
#ifdef DEBUG
      printf ("bytes_read = %i, tail at %li\n", bytes_read, tail-buf);
#endif
      parse_csv (buf, &tail, !bytes_read, &in_comment, &current_row_idx, &current_col_idx, 0, &new_value, &new_comment);
    }
  while (bytes_read);
  putchar ('\n');
  free (buf);
  if (fd != STDIN_FILENO)
    close (fd);
  return 0;
}
