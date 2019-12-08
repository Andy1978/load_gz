//#include <cstdlib>
//#include <ostream>
#include <iostream>
#include <string>

//#include <octave/lo-mappers.h>
//#include <octave/lo-utils.h>
//#include <octave/mx-base.h>
//#include <octave/str-vec.h>

#include <octave/defun-dld.h>
#include <octave/errwarn.h>
#include <octave/interpreter.h>
//#include <octave/ops.h>
#include <octave/ov-base.h>
//#include <octave/ov-scalar.h>
//#include <octave/ov-typeinfo.h>
//#include <octave/ov.h>
//#include <octave/ovl.h>
//#include <octave/pager.h>
//#include <octave/pr-output.h>
//#include <octave/variables.h>

#include "zlib.h"

#define INITIAL_ROWS 100
#define GROWTH_FACTOR 1.5
// BUFFER_SIZE has to be at least the maximum rowlength in bytes
#define BUFFER_SIZE 8000

class load_gz : public octave_base_value
{
public:

  load_gz (void)
    : octave_base_value (),
      gz_fid (NULL),
      empty_val (lo_ieee_na_value ()),
      current_row_idx (0)
  { }

  load_gz (const std::string &fn)
    : octave_base_value (),
      gz_fid (NULL),
      empty_val (lo_ieee_na_value ()),
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

  load_gz (const load_gz& s)
    : octave_base_value () { }

  ~load_gz (void)
  {
    //std::cout << "load_gz destructor" << std::endl;
    free (buf);
    if (gz_fid)
      gzclose (gz_fid);
  }

  octave_base_value * clone (void)
  {
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
    return true;
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

  void print (std::ostream& os, bool pr_as_read_syntax = false);

private:

  std::string gz_fn;
  gzFile gz_fid;

  double empty_val;                   // currently NA (FIXME: make it configurable)?

  Matrix mat;
  octave_idx_type current_row_idx;

  char *buf;
  char *head;

  int poll ()
  {
    if (gzeof (gz_fid))
      {
        //fprintf (stderr, "reset EOF...\n");
        gzclearerr (gz_fid);
        return 0;
      }

    int bytes_read = gzread (gz_fid, head, BUFFER_SIZE - (head - buf) - 1);
    head[bytes_read] = 0;

    //fprintf (stderr, "bytes_read = %i\n", bytes_read);
    //fprintf (stderr, "buf after gzread = '%s'\n", buf);

    //for (int k = 0; k < BUFFER_SIZE; ++k)
    //  fprintf (stderr, "buf[%i] = 0x%X = '%c'\n", k, buf[k], buf[k]);

    if (bytes_read > 0)
      {
        char *tail = head + bytes_read;

        //fprintf (stderr, "columns = %i\n", mat.columns());

        char *start = buf;
        char *end = buf;

        /* Idee:
         * Parsen, bis ein Zeichen kommt, welches von strtod nicht interpretiert werden kann.
         * Somit kann ein Trennzeichen auch ein ";" oder "," sein
         *
         * buf fängt immer mit dem Anfang der Zeile an, wurde beim letzten Mal pollen dier Zeile nicht mit LF abgeschlossen,
         * so bleibt sie im buffer
         */

        octave_idx_type current_col_idx = 0;
        while (start < tail)
          {
            if (*start == '#')
              while (*start++ != 0x0A);

            double d = strtod (start, &end);

            //if (end == start)
            //  fprintf (stderr, "ungültiges zeichen\n");

            //fprintf (stderr, "d = %f, char at end is = 0x%X\n", d, *end);

            //fprintf (stderr, "c = %i, r = %i, d = %f, *end = 0x%X\n", current_col_idx, current_row_idx, d, *end);

            if (current_col_idx >= mat.columns ())
              mat.resize (mat.rows(), current_col_idx + 1, empty_val);

            // end == start kann z.B. Auftreten, wenn ungültige Zeichen
            // oder zwei "nicht whitespace" Trennzeichen, z.B. 5;;6
            if (end != start)
              mat (current_row_idx, current_col_idx) = d;

            start = end + 1;

            current_col_idx++;
            if (*end == 0x0A)
              {
                //fprintf (stderr, "newline\n");

                current_row_idx++;
                if (mat.rows () < current_row_idx + 1)
                  {
                    mat.resize (mat.rows () * GROWTH_FACTOR, mat.columns (), empty_val);
                    //fprintf (stderr, "rows after resize = %i\n", mat.rows ());
                  }
                current_col_idx = 0;
                head = start;
              }

          }
        int chars_left = tail - head;
        //fprintf (stderr, "exit while, %i bytes left\n", chars_left);

        //fprintf (stderr, "head before move = '%s'\n", head);

        // umkopieren (mit trailing 0)
        memmove (buf, head, chars_left + 1);
        head = buf + chars_left;
        current_col_idx = 0;

        //fprintf (stderr, "buf after move = '%s'\n", buf);
      }
    return bytes_read;
  }

  DECLARE_OV_TYPEID_FUNCTIONS_AND_DATA
};

void load_gz::print (std::ostream& os, bool pr_as_read_syntax)
{
  os << "class load_gz:\n";
  os << "\n  gz_fn   = '" << gz_fn.c_str () << "'";
  os << "\n  gz_fid  = " << gz_fid;
  os << "\n  rows  = " << rows();
  os << "\n  columns  = " << columns();
  newline (os);
}

DEFMETHOD_DLD (load_gz, interp, args,,
               "mat = load_gz (fn)\n\
\n\
Reads matrix from VAL.")
{
  static bool type_loaded = false;

  if (! type_loaded)
    {
      load_gz::register_type ();
      interp.mlock ();

      //octave_stdout << "installing load_gz at type-id = "
      //              << load_gz::static_type_id () << "\n";

      octave::type_info& ti = interp.get_type_info ();

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
      retval.append (((load_gz&) rep) . matrix_value ());
    }
  else
    err_wrong_type_arg ("mget", args(0));

  return retval;
}

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA (load_gz, "load_gz", "load_gz");

/*
%!test
%! m = rand (5e5, 8);
%! fn = tempname();
%! save ("-z", "-ascii", fn, "m")
%! tic; ref = load (fn); t_load = toc()
%! tic; x = load_gz (fn); m = mget (x); t_load_gz = toc()
%! printf ("speed up is %.1f\n", t_load/t_load_gz);
%! assert (m, ref);
%! unlink (fn);
*/
