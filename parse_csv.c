#include "parse_csv.h"

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
 * so that the next read can start writind data to *tail.
 */

void parse_csv (char *buf,
                char** tail,
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

  while (start < *tail)
    {
      if (*current_col_idx == 0 && *start == '#')
        {
          // comment char at beginning of line
          DBG_STR ("comment char at beginning of line");


// FIXME: Ich denke ich werde das so implementieren,
// dass ein comment am Stück in den Buffer passen muss. Wenn das nicht der Fall ist,
// wird er halt abgeschnitten.

new_comment (userdata, "foobar");



          *in_comment = 1;
        }

      double d = strtod (start, &end);

      if (end == start)
        {
          // no conversion was performed
          DBG_STR ("no conversion was performed");
          start = end + 1;

        }
      else if (end == *tail)
        {
          // possible premature end of conversion due to end of buffer
          DBG_STR ("possible premature end of conversion due to end of buffer");
          break;

        }
      else
        {
          // All fine, store value into mat
          #ifdef DEBUG
          printf ("All fine, store value '%f' into mat (%i, %i)\n", d, *current_row_idx, *current_col_idx);
          #endif
          new_value (userdata, *current_row_idx, *current_col_idx, d);
          start = end + 1;
        }

      (*current_col_idx)++;

      if (isEOL (*end))
        {
          (*current_row_idx)++;
          *current_col_idx = 0;

          // consume as much of EOL chars as possible
          while (++end < *tail && isEOL (*end))
            DBG_STR ("skip EOL\n");

          start = end;
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
