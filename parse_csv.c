#include "parse_csv.h"

char isEOL (char c)
{
  return c == 0x0A || c == 0x0D;
}
  
void parse (char *buf, char** tail)
{
  static int current_row_idx = 0;
  static int current_col_idx = 0;
  static char in_comment = 0;

  if (!buf) // init
    {
      current_row_idx = current_col_idx = in_comment = 0;
      return;
    }
  
  // tail points one byte past the last char read (to trailing \0)
  assert (**tail == 0);

  char *start = buf;
  char *end = buf;

  while (start < *tail)
    {
      if (current_col_idx == 0 && *start == '#')
        {
          // comment char at beginning of line
          printf ("comment char at beginning of line\n");

          in_comment = 1;
        }

      double d = strtod (start, &end);

      if (end == start)
        {
          // no conversion was performed
          printf ("no conversion was performed\n");
          start = end + 1;

        }
      else if (end == *tail)
        {
          // possible premature end of conversion due to end of buffer
          printf ("possible premature end of conversion due to end of buffer\n");
          break;

        }
      else
        {
          // All fine, store value into mat
          printf ("All fine, store value '%f' into mat (%i, %i)\n", d, current_row_idx, current_col_idx);
          cb_new_value (current_row_idx, current_col_idx, d);
          start = end + 1;
        }

      current_col_idx++;

      if (isEOL (*end))
        {
          current_row_idx++;
          current_col_idx = 0;

          // consume as much of EOL chars as possible
          while (++end < *tail && isEOL (*end))
            printf ("skip EOL\n");

          start = end;
        }



    } // end while

    int chars_left = *tail - start;
    printf ("chars_left = %i, '%s'\n", chars_left, start);
    
    // move memory from start to tail to buf
    // (remove converted parts, move remaining/not yet used memory to beginning of buffer)
    memmove (buf, start, chars_left + 1);
    *tail = buf + chars_left;

    printf ("buf = '%s'\n", buf);

}
