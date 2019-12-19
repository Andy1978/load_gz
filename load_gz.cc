/*
  load_gz: An GNU Octave wrapper to (optional incrementally) load
  numerical matrices.

  Copyright (C) 2019 Andreas Weber

  This program is free software; you can redistribute it and/or
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

#include <iostream>
#include "zlib.h"

#include <octave/oct.h>
#include <octave/defun-dld.h>

#if (OCTAVE_MAJOR_VERSION == 4 && OCTAVE_MINOR_VERSION < 4) || OCTAVE_MAJOR_VERSION < 4

#else
  #include <octave/interpreter.h>
#endif

#include <octave/ov-base.h>

/***********************    settings    ********************************/
#define INITIAL_ROWS 100
#define GROWTH_FACTOR 1.5
// BUFFER_SIZE should be at least >20 and >1000 for good performance
#define BUFFER_SIZE 25
/************************  class load_gz  ******************************/

#include "parse_csv.h"

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
    DBG_STR ("c'tor");

    gz_fn = fn;
    gz_fid = gzopen (gz_fn.c_str (), "r");
    if (! gz_fid)
      error ("Opening '%s' failed", gz_fn.c_str ());

    buf = tail = (char *) malloc (BUFFER_SIZE);
    if (! buf)
      error ("malloc failed");

    mat = Matrix (INITIAL_ROWS, 1, empty_val);
  }

  //~ load_gz (const load_gz& s)
    //~ : octave_base_value () { }

  ~load_gz (void)
  {
    DBG_STR ("d'tor");
    free (buf);
    if (gz_fid)
      gzclose (gz_fid);
  }

  octave_base_value * clone (void)
  {
    DBG_STR ("");
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

  char in_comment;                  // start of comment detected but now newline yet
  int current_col_idx;
  int current_row_idx;

  char *buf;                        // internal buffer of size BUFFER_SIZE
  char *tail;                       // one byte behind the end of read data

  bool isEOL (char c)
  {
    return c == 0x0A || c == 0x0D;
  }

  static void cb_wrap_new_value (void *p, int row, int col, double value)
  {
    ((load_gz*) p)->new_value (row, col, value);
  }

  static void cb_wrap_new_comment (void *p, char append, char complete, const char* c)
  {
    ((load_gz*) p)->new_comment (append, complete, c);
  } 

  void new_value (int row, int col, double value)
  {
    // handle resizing here
    #ifdef DEBUG
    printf ("mats(%i,%i) = %f\n", row, col, value);
    #endif

    // Check if we need to increase the number of columns
    if (col >= mat.columns ())
      mat.resize (mat.rows(), col + 1, empty_val);

    // Check if we need to increase the number of rows
    if (row >= mat.rows ())
      mat.resize (mat.rows () * GROWTH_FACTOR, mat.columns (), empty_val);
    
    mat (row, col) = value;
  }

  void new_comment (char append, char, const char* c)
  {
    #ifdef DEBUG
    printf ("new_comment append = %i, complete = %i, '%s'\n", append, complete, c);
    #endif
    if (! append)
      {
        comments.resize (dim_vector (comments.rows () + 1, 1));
        comments(comments.rows () - 1) = std::string (c);
      }
    else
      comments(comments.rows () - 1) = comments(comments.rows () - 1).string_value() + std::string (c);
  }

  int poll ()
  {
    if (gzeof (gz_fid))
      {
        DBG_STR ("gzclearerr (gz_fid)");
        gzclearerr (gz_fid);
        return 0;
      }

    int bytes_read = gzread (gz_fid, tail, BUFFER_SIZE - (tail - buf) - 1);
    tail = tail + bytes_read;
    *tail = 0;

    DBG_INT_VAL (bytes_read);

//    for (int k = 0; k < (head + bytes_read - buf + 1); ++k)
//      printf ("DEBUG: buf[%i] = 0x%X = '%c'\n", k, buf[k], buf[k]);

    if (bytes_read > 0)
      {
        parse_csv (buf, &tail, &in_comment, &current_row_idx, &current_col_idx, this, &cb_wrap_new_value, &cb_wrap_new_comment);
      }
    return bytes_read;
  }

  DECLARE_OV_TYPEID_FUNCTIONS_AND_DATA
};

void load_gz::print (std::ostream& os, bool)// pr_as_read_syntax)
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
%! unlink (fn);

*/
