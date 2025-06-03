/*
  load_xz: An GNU Octave wrapper to (optional incrementally) load
  numerical matrices.

  Copyright (C) 2025 Andreas Weber

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
#include <lzma.h>

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
// BUFFER_SIZE should be at least >20 and >500 for good performance
#define BUFFER_SIZE 20
/************************  class load_xz  ******************************/

#include "parse_csv.h"

class load_xz : public octave_base_value
{
public:

  load_xz (void)
    : octave_base_value (),
      strm (LZMA_STREAM_INIT),
      fid (NULL),
      empty_val (lo_ieee_na_value ()),
      in_comment (false),
      current_col_idx (0),
      current_row_idx (0)
  { }

  load_xz (const std::string &fn)
    : octave_base_value (),
      strm (LZMA_STREAM_INIT),
      fid (NULL),
      empty_val (lo_ieee_na_value ()),
      in_comment (false),
      current_col_idx (0),
      current_row_idx (0)
  {
    DBG_STR ("c'tor");

    strm = LZMA_STREAM_INIT;
    lzma_ret ret = lzma_stream_decoder (&strm, UINT64_MAX, LZMA_CONCATENATED);

    // Return successfully if the initialization went fine.
    if (ret != LZMA_OK)
      error ("init_decoder failed ret = %i", ret);

    xz_fn = fn;
    /*
     * FIXME: this doesn't work with ~ expansion which is normally done by SHELL
     * See https://linux.die.net/man/3/wordexp
     */
    fid = fopen (xz_fn.c_str (), "rb");

    if (! fid)
      error ("Opening '%s' failed", xz_fn.c_str ());

    buf = tail = (char *) malloc (BUFFER_SIZE);
    if (! buf)
      error ("malloc failed");

    mat = Matrix (INITIAL_ROWS, 1, empty_val);
  }

  //~ load_xz (const load_gz& s)
  //~ : octave_base_value () { }

  ~load_xz (void)
  {
    DBG_STR ("d'tor");
    free (buf);
    if (fid)
      fclose (fid);
  }

  octave_base_value * clone (void)
  {
    DBG_STR ("");
    return new load_xz (*this);
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
    poll ();

    if (current_row_idx > 0)
      return mat.extract (0, 0, current_row_idx - 1, mat.columns () - 1);
    else
      return Matrix (0, 0);
  }

  octave_value get () const
  {
    octave_scalar_map retval;

    retval.assign ("fn", xz_fn);
    retval.assign ("rows", rows());
    retval.assign ("allocated_rows", mat.rows());
    retval.assign ("columns", mat.columns());
    retval.assign ("comments", comments);

    return retval;
  }

  void print (std::ostream& os, bool pr_as_read_syntax = false);

private:

  std::string xz_fn;
  lzma_stream strm;
  FILE *fid;

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
    ((load_xz*) p)->new_value (row, col, value);
  }

  static void cb_wrap_new_comment (void *p, char append, char complete, const char* c)
  {
    ((load_xz*) p)->new_comment (append, complete, c);
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

  void new_comment (char append, char complete, const char* c)
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

  void poll ()
  {
    if (feof (fid))
      {
        DBG_STR ("feof == true");
        // TODO: should we return here?
      }
    // based on https://github.com/tukaani-project/xz/blob/master/doc/examples/02_decompress.c#L103
    lzma_action action = LZMA_RUN;

    uint8_t inbuf[BUFSIZ];

    strm.next_in = NULL;
    strm.avail_in = 0;

    while (true)
      {
        if (strm.avail_in == 0 && !feof(fid))
          {
            strm.next_in = inbuf;
            strm.avail_in = fread(inbuf, 1, sizeof(inbuf), fid);
            printf ("DEBUG: strm.avail_in = %i\n", strm.avail_in);

            if (ferror (fid))
              error ("Read error: %s\n", strerror(errno));

            // Once the end of the input file has been reached,
            // we need to tell lzma_code() that no more input
            // will be coming. As said before, this isn't required
            // if the LZMA_CONCATENATED flag isn't used when
            // initializing the decoder.
            if (feof (fid))
              action = LZMA_FINISH;
          }

        strm.next_out = (uint8_t*) tail;
        strm.avail_out = BUFFER_SIZE - (tail - buf) - 1;
        printf ("DEBUG: strm.avail_out = %i\n", strm.avail_out);
        
        
        
        lzma_ret ret = lzma_code(&strm, action);

        printf ("DEBUG: strm.avail_out = %i\n", strm.avail_out);
        printf ("DEBUG: lzma_code returned = %i\n", ret);

        if (strm.avail_out == 0 || ret == LZMA_STREAM_END)
          {
            size_t write_size = BUFFER_SIZE - (tail - buf) - 1 - strm.avail_out;
            printf ("DEBUG: write_size = %i\n", write_size);
            tail = tail + write_size;
            *tail = 0;

            //if (fwrite(outbuf, 1, write_size, stdout) != write_size)
            //  error ("Write error: %s", strerror(errno));

            parse_csv (buf, &tail, !write_size, &in_comment, &current_row_idx, &current_col_idx, this, &cb_wrap_new_value, &cb_wrap_new_comment);
          }

        if (ret != LZMA_OK)
          {
            if (ret == LZMA_STREAM_END) // this is okay
              return;
            else
              error ("lzma_code returned %i", ret);
          }
      }
  }

  DECLARE_OV_TYPEID_FUNCTIONS_AND_DATA
};

void load_xz::print (std::ostream& os, bool)// pr_as_read_syntax)
{
  os << "class load_xz:\n";
  os << "\n  fn       = '" << xz_fn.c_str () << "'";
  os << "\n  rows     = " << rows();
  os << "\n  columns  = " << columns();
  for (unsigned int k = 0; k < comments.rows(); ++k)
    os << "\n  comments(" << k << ") = " << comments(k).string_value();
  newline (os);
}

#if (OCTAVE_MAJOR_VERSION == 4 && OCTAVE_MINOR_VERSION < 4) || OCTAVE_MAJOR_VERSION < 4
DEFUN_DLD (load_xz, args,,
#else
DEFMETHOD_DLD (load_xz, interp, args,,
#endif
           "mat = load_xz (fn)\n\
\n\
Reads matrix from previously with load_xz opened file.")
{
  static bool type_loaded = false;

  if (! type_loaded)
    {
      load_xz::register_type ();
#if (OCTAVE_MAJOR_VERSION == 4 && OCTAVE_MINOR_VERSION < 4) || OCTAVE_MAJOR_VERSION < 4
      mlock ();
#else
      interp.mlock ();
#endif
      //octave_stdout << "installing load_xz at type-id = "
      //              << load_xz::static_type_id () << "\n";

      //octave::type_info& ti = interp.get_type_info ();

      type_loaded = true;
    }

  octave_value retval;

  if (args.length () == 1)
    {
      std::string fn = args(0).string_value ();

      retval = octave_value (new load_xz (fn));
    }
  else
    print_usage ();

  return retval;
}

//autoload ("mget", which ("load_xz.oct"))
DEFUN_DLD (mget, args,,
           "mget (I)")
{
  octave_value_list retval;
  if (args.length () != 1)
    {
      print_usage();
      return retval;
    }
  if (args(0).type_id () == load_xz::static_type_id ())
    {
      const octave_base_value& rep = args(0).get_rep ();
      retval.append (((load_xz&) rep).matrix_value ());
    }
  else
#if (OCTAVE_MAJOR_VERSION == 4 && OCTAVE_MINOR_VERSION < 2) || OCTAVE_MAJOR_VERSION < 4
    gripe_wrong_type_arg ("mget", args(0));
#else
    err_wrong_type_arg ("mget", args(0));
#endif

  return retval;
}

DEFUN_DLD (xget, args,,
           "xget (I)")
{
  octave_value_list retval;
  if (args.length () != 1)
    {
      print_usage();
      return retval;
    }
  if (args(0).type_id () == load_xz::static_type_id ())
    {
      const octave_base_value& rep = args(0).get_rep ();
      retval.append(((load_xz&) rep).get ());
    }
  else
#if (OCTAVE_MAJOR_VERSION == 4 && OCTAVE_MINOR_VERSION < 2) || OCTAVE_MAJOR_VERSION < 4
    gripe_wrong_type_arg ("get", args(0));
#else
    err_wrong_type_arg ("get", args(0));
#endif

  return retval;
}

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA (load_xz, "load_xz", "load_xz");

// TODO: Die Tests aus load_gz funktionieren so nicht...
