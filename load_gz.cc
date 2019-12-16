#include <iostream>
#include "zlib.h"

#include <octave/oct.h>
#include <octave/defun-dld.h>

#if (OCTAVE_MAJOR_VERSION == 4 && OCTAVE_MINOR_VERSION < 4) || OCTAVE_MAJOR_VERSION < 4

#else
  #include <octave/interpreter.h>
#endif

#include <octave/ov-base.h>

#define INITIAL_ROWS 100
#define GROWTH_FACTOR 1.5
// BUFFER_SIZE has to be at least the maximum rowlength in bytes
#define BUFFER_SIZE 10

//#define DEBUG

class load_gz : public octave_base_value
{
public:

  load_gz (void)
    : octave_base_value (),
      gz_fid (NULL),
      empty_val (lo_ieee_na_value ()),
      in_comment (false),
      current_col_idx (0),
      current_row_idx (0)
  { }

  load_gz (const std::string &fn)
    : octave_base_value (),
      gz_fid (NULL),
      empty_val (lo_ieee_na_value ()),
      in_comment (false),
      current_col_idx (0),
      current_row_idx (0)
  {
    gz_fn = fn;
    gz_fid = gzopen (gz_fn.c_str (), "r");
    if (! gz_fid)
      error ("Opening '%s' failed", gz_fn.c_str ());

    buf = head = (char *) malloc (BUFFER_SIZE);
    if (! buf)
      error ("malloc failed");

    mat = Matrix (INITIAL_ROWS, 1, empty_val);
  }

  //~ load_gz (const load_gz& s)
    //~ : octave_base_value () { }

  ~load_gz (void)
  {
#ifdef DEBUG
    std::cout << "DEBUG: load_gz destructor" << std::endl;
#endif
    free (buf);
    if (gz_fid)
      gzclose (gz_fid);
  }

  octave_base_value * clone (void)
  {
#ifdef DEBUG
    std::cout << "DEBUG: load_gz clone" << std::endl;
#endif
    return new load_gz (*this);
  }

  int rows (void) const
  {
    return current_row_idx;
  }

  int columns (void) const
  {
    return mat.columns ();
  }

  bool is_constant (void) const
  {
    return false;
  }

  bool is_defined (void) const
  {
    return true;
  }

  Matrix matrix_value (bool = false)
  {
    while (poll ());

    if (current_row_idx > 0)
      return mat.extract (0, 0, current_row_idx - 1, mat.columns () - 1);
    else
      return Matrix (0, 0);
  }

  octave_value get () const
  {
    octave_scalar_map retval;

    retval.assign ("fn", gz_fn);
    retval.assign ("rows", rows());
    retval.assign ("allocated_rows", mat.rows());
    retval.assign ("columns", mat.columns());
    retval.assign ("comments", comments);

    return retval;
  }

  void print (std::ostream& os, bool pr_as_read_syntax = false);

private:

  std::string gz_fn;
  gzFile gz_fid;

  double empty_val;                  // currently NA (FIXME: make it configurable)?

  Cell comments;
  Matrix mat;

  bool in_comment;                   // start of comment detected but now newline yet
  octave_idx_type current_col_idx;
  octave_idx_type current_row_idx;

  char *buf;                        // internal buffer of size BUFFER_SIZE
  char *head;                       // read position into internal buffer

  bool isEOL (char c)
  {
    return c == 0x0A || c == 0x0D;
  }

  int poll ()
  {
#ifdef DEBUG
    //std::cout << "DEBUG: gzeof (gz_fid) = " << gzeof (gz_fid) << std::endl;
#endif
    if (gzeof (gz_fid))
      {
#ifdef DEBUG
        std::cout << "DEBUG: gzclearerr (gz_fid)" << std::endl;
#endif
        gzclearerr (gz_fid);
        return 0;
      }

    int bytes_read = gzread (gz_fid, head, BUFFER_SIZE - (head - buf) - 1);
    head[bytes_read] = 0;

#ifdef DEBUG
    std::cout << "DEBUG: poll bytes_read = " << bytes_read << std::endl;

    for (int k = 0; k < (head + bytes_read - buf + 1); ++k)
      printf ("DEBUG: buf[%i] = 0x%X = '%c'\n", k, buf[k], buf[k]);
#endif

    if (bytes_read > 0)
      {
        // tail points one byte past the last char read (to trailing \0)
        char *tail = head + bytes_read;
        assert (*tail == 0);

        char *start = buf;
        char *end = buf;

        /* How parsing works:
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
         */

        while (start < tail)
          {
            // # indicates comment -> consume char until EOL
            if (*start == '#')
              {
                char *start_of_comment = start;
                while (start < tail && ! isEOL (*start++));

                // check if the buffer ended before an EOL was found
                if (! *start)
                  {
                    printf ("buffer ended before EOL\n");
                    // Set head at start of comment and bail out
                    head = start_of_comment;
                    break;
                  }
                else
                  {
                    std::string tmp (start_of_comment, start - start_of_comment - 1);
                    //printf ("saving comment '%s'\n", tmp.c_str());
                    comments.resize (dim_vector (comments.rows () + 1, 1));
                    comments(comments.rows () - 1) = tmp;
                    head = start;
                  }

                in_comment = true;
              }

            double d = strtod (start, &end);

            //if (end == start)
            //  fprintf (stderr, "non-convertible char\n");
            //fprintf (stderr, "d = %f, char at end is = 0x%X\n", d, *end);
            //fprintf (stderr, "c = %i, r = %i, d = %f, *end = 0x%X\n", current_col_idx, current_row_idx, d, *end);

            // Check if we need to increase the number of columns
            if (current_col_idx >= mat.columns ())
              mat.resize (mat.rows(), current_col_idx + 1, empty_val);

            // strtod conversion was successful;
            if (end != start)
              {
                row_had_data = true;
                mat (current_row_idx, current_col_idx) = d;
              }

            start = end + 1;

            current_col_idx++;

            if (row_had_data && isEOL (*end))
              {
                current_row_idx++;
                if (mat.rows () < current_row_idx + 1)
                  mat.resize (mat.rows () * GROWTH_FACTOR, mat.columns (), empty_val);

                current_col_idx = 0;
                head = start;
                row_had_data = false;
              }

          }
        int chars_left = tail - head;

        // remove converted parts (move remaining buf to left)
        memmove (buf, head, chars_left + 1);
        head = buf + chars_left;
        current_col_idx = 0;

      }
    return bytes_read;
  }

  DECLARE_OV_TYPEID_FUNCTIONS_AND_DATA
};

void load_gz::print (std::ostream& os, bool pr_as_read_syntax)
{
  os << "class load_gz:\n";
  os << "\n  fn       = '" << gz_fn.c_str () << "'";
  os << "\n  rows     = " << rows();
  os << "\n  columns  = " << columns();
  for (unsigned int k = 0; k < comments.rows(); ++k)
    os << "\n  comments(" << k << ") = " << comments(k).string_value();
  newline (os);
}

#if (OCTAVE_MAJOR_VERSION == 4 && OCTAVE_MINOR_VERSION < 4) || OCTAVE_MAJOR_VERSION < 4
DEFUN_DLD (load_gz, args, ,
#else
DEFMETHOD_DLD (load_gz, interp, args,,
#endif
               "mat = load_gz (fn)\n\
\n\
Reads matrix from previously with load_gz opened file.")
{
  static bool type_loaded = false;

  if (! type_loaded)
    {
      load_gz::register_type ();
#if (OCTAVE_MAJOR_VERSION == 4 && OCTAVE_MINOR_VERSION < 4) || OCTAVE_MAJOR_VERSION < 4
      mlock ();
#else
      interp.mlock ();
#endif
      //octave_stdout << "installing load_gz at type-id = "
      //              << load_gz::static_type_id () << "\n";

      //octave::type_info& ti = interp.get_type_info ();

      type_loaded = true;
    }

  octave_value retval;

  if (args.length () == 1)
    {
      std::string fn = args(0).string_value ();

      retval = octave_value (new load_gz (fn));
    }
  else
    print_usage ();

  return retval;
}

//autoload ("mget", which ("load_gz.oct"))
DEFUN_DLD (mget, args,,
           "mget (I)")
{
  octave_value_list retval;
  if (args.length () != 1)
    {
      print_usage();
      return retval;
    }
  if (args(0).type_id () == load_gz::static_type_id ())
    {
      const octave_base_value& rep = args(0).get_rep ();
      retval.append (((load_gz&) rep).matrix_value ());
    }
  else
#if (OCTAVE_MAJOR_VERSION == 4 && OCTAVE_MINOR_VERSION < 2) || OCTAVE_MAJOR_VERSION < 4
    gripe_wrong_type_arg ("mget", args(0));
#else
    err_wrong_type_arg ("mget", args(0));
#endif

  return retval;
}

DEFUN_DLD (get, args,,
           "get (I)")
{
  octave_value_list retval;
  if (args.length () != 1)
    {
      print_usage();
      return retval;
    }
  if (args(0).type_id () == load_gz::static_type_id ())
    {
      const octave_base_value& rep = args(0).get_rep ();
      retval.append(((load_gz&) rep).get ());
    }
  else
#if (OCTAVE_MAJOR_VERSION == 4 && OCTAVE_MINOR_VERSION < 2) || OCTAVE_MAJOR_VERSION < 4
    gripe_wrong_type_arg ("get", args(0));
#else
    err_wrong_type_arg ("get", args(0));
#endif

  return retval;
}

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA (load_gz, "load_gz", "load_gz");

/*
%!test
%! m = rand (5e4, 8);
%! fn = tempname();
%! save ("-z", "-ascii", fn, "m")
%! tic; ref = load (fn); t_load = toc()
%! tic; x = load_gz (fn); m = mget (x); t_load_gz = toc()
%! printf ("speed up is %.1f\n", t_load/t_load_gz);
%! assert (m, ref);
%! unlink (fn);

%!test
%! fn = tempname();
%!
%! fid = fopen (fn, "wb");
%!
%! fprintf (fid, "# testing mixed delimiter and linebreaks\n");
%! fprintf (fid, "#\n");
%! fprintf (fid, "7 8.12 9.333\n");
%! fprintf (fid, "1;2.3,4.5\n");
%! fprintf (fid, "\n");
%! fprintf (fid, "2\t3.4\t5.6\n");
%! fprintf (fid, "# CR linebreak (classic apple)\r");
%! fprintf (fid, "10 20 30\r");
%! fprintf (fid, "15 25 35.6\r");
%! fprintf (fid, "# CR+LF linebreak (windoze)\r\n");
%! fprintf (fid, "2.1255363456 3.123467384456 4.874443367876\r\n");
%! fprintf (fid, "\r\n");
%! fprintf (fid, "3.14156 2.718 40\r\n");
%! fclose (fid);
%!
%! x = load_gz (fn);
%! m = mget (x);
%!
%! ref = [7 8.12 9.333;
%!        1 2.3 4.5;
%!        2 3.4 5.6;
%!        10 20 30;
%!        15 25 35.6;
%!        2.1255363456 3.123467384456 4.874443367876;
%!        3.14156 2.718 40];
%!
%! assert (m, ref);
%!
%! s = get (x);
%! assert (s.fn, fn)
%! assert (s.rows, 7)
%! assert (s.columns, 3)
%! assert (s.comments{1}, "# testing mixed delimiter and linebreaks")
%! assert (s.comments{2}, "#")
%! assert (s.comments{3}, "# CR linebreak (classic apple)")
%! assert (s.comments{4}, "# CR+LF linebreak (windoze)")
%!
%! unlink (fn);

*/
