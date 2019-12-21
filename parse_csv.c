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

#include "parse_csv.h"
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

char isEOL (char c)
{
  return c == 0x0A || c == 0x0D;
}

/* How parsing works:
 *
 * *buf points to the start of allocated memory,
 * *tail one byte behind the end of data in the buffer.
 *
 * strtod reads until it hits a non-convertible character.
 * The read double is written into the matrix (if there conversion was successful)
 * and the column pointer is incremented.
 * If the next char is a newline (CR || LF, see isEOL), the next row is addressed
 * and the column index is set to 0. Subsequent newline chars are ignored.
 *
 * With this it's possible to mix single column delimiters, for example
 * "4 5.6;7.8,9"
 *
 * If there are two non-convertible chars, the empty val (currently only NA) is used.
 * -> "4;;5;6" results in a matrix [4 NA 5 6]
 *
 * Non-processed data is moved to *buf at the end and *tail is updated accordingly
 * so that the next read can start writing data to *tail.
 */

void parse_csv (char *buf,
                char** tail,
                char flush,
                char *in_comment,
                int *current_row_idx,
                int *current_col_idx,
                void* userdata,
                cb_new_value new_value,
                cb_new_comment new_comment)
{
  // tail points one byte past the last char read (to trailing \0)
  assert (**tail == 0);

  char *start = buf;
  char *end = buf;

  //printf ("flush = %i\n", flush);
  int min_len = (flush)? 0 : 10;

  while ((*tail - start) > min_len)
    {
      //printf ("*start = %i = '%c', start = '%s'\n", *start, *start, start);
      if ((*current_col_idx == 0 && *start == '#') || *in_comment)
        {
          // new comment or already in "read comment" state
          DBG_INT_VAL (*in_comment);

          // find EOL
          end = start;
          while (*(end) && !isEOL (*end)) end++;
          DBG_LINT_VAL (end-buf);

          if (! *end) // no EOL found yet
            {
              DBG_STR ("parsing comment: buffer ended before EOL");
              new_comment (userdata, *in_comment, 0, start);
              *in_comment = 1;
              start = end;
              // bail out
              break;
            }

          // an EOL was found. Overwrite it with \0
          *end = 0;

          new_comment (userdata, *in_comment, 1, start);
          *in_comment = 0;

          // consume as much of EOL chars as possible
          //while (++end < *tail && isEOL (*end))
          //  DBG_STR ("skip EOL");

          start = end + 1;
        }
      else if (isEOL (*start))
        {
          DBG_STR ("isEOL");
          if (*current_col_idx > 0)
            {
              (*current_row_idx)++;
              *current_col_idx = 0;
            }
          start++;
        }
      else  // "normal" string to double conversion
        {
          DBG_STR ("strtod");
          //printf ("*start = %i = '%c', start = '%s'\n", *start, *start, start);

          // normal case: (see test_strtod for tests)
          // start = '1.23', d = 1.230000, end-start = 4, *end = 0 = ''
          // whistepace at start is ignored:
          // start = ' 2.34 ', d = 2.340000, end-start = 5, *end = 32 = ' '
          // Only whitespace before end of buffer
          // start = '  ', d = 0.000000, end-start = 0, *end = 32 = ' '
          // Non-convertible at start
          // start = ';3.14', d = 0.000000, end-start = 0, *end = 59 = ';'
          // Not finished expotential
          // start = '9e', d = 9.000000, end-start = 1, *end = 101 = 'e'

          double d = strtod (start, &end);

          if (end == start)
            {
              DBG_STR ("no conversion was performed");
              if (! isspace (*start))
                (*current_col_idx)++;
              start++;
            }
          else if (! *end || *end == 'e' || *end == 'E' || *end == 'x' || *end == 'X')
            {
              DBG_STR ("possible premature end of conversion due to end of buffer");
              break;
            }
          else // All fine, store value into mat
            {
              new_value (userdata, *current_row_idx, *current_col_idx, d);
              start = end;
              if (! isEOL (*start))
                start++;
              (*current_col_idx)++;
            }
        }
    } // end while

  int chars_left = *tail - start;
  DBG_INT_VAL (chars_left);

  // move memory from start to tail to buf
  // (remove converted parts, move remaining/not yet used memory to beginning of buffer)
  memmove (buf, start, chars_left + 1);
  *tail = buf + chars_left;

  DBG_STR_VAL (buf);
}
