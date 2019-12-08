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

#define INITIAL_ROWS 2
#define GROWTH_FACTOR 1.5
// has to be at least maximum rowlength in bytes
#define BUFFER_SIZE 60

class load_gz : public octave_base_value
{
public:

  load_gz (void)
    : octave_base_value (),
      scalar (0),
      gz_fid (NULL)
  { }

  load_gz (const std::string &fn)
    : octave_base_value (),
      scalar (0),
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
    : octave_base_value (), scalar (s.scalar) { }

  ~load_gz (void)
  {
    std::cout << "load_gz destructor" << std::endl;
    free (buf);
    if (gz_fid)
      gzclose (gz_fid);
  }

  octave_base_value * clone (void) { return new load_gz (*this); }

  int rows (void) const { return 3; }
  int columns (void) const { return 4; }

  bool is_constant (void) const { return true; }
  bool is_defined (void) const { return true; }

  Matrix matrix_value (bool = false)
  {
    int bytes_read = gzread (gz_fid, head, BUFFER_SIZE - (head - buf) - 1);
    if (bytes_read > 0)
      {
      buf[bytes_read] = 0;
      char *tail = head + bytes_read;

      fprintf (stderr, "bytes_read = %i\n", bytes_read);
      fprintf (stderr, "columns = %i\n", mat.columns());

      char *start = buf;
      char *end = buf;

      /* Idee:
       * Parsen, bis ein Zeichen kommt, welches von strtod nicht interpretiert werden kann.
       * Somit kann ein Trennzeichen auch ein ";" oder "," sein
       * 
       * buf fängt immer mit dem Anfang der Zeile an, wurde beim letzten Mal pollen dier Zeile nicht mit LF abgeschlossen,
       * so bleibt sie im buffer
       */
      
      int c = 0;
      while (start < tail)
        {
          double d = strtod (start, &end);

          printf ("d = %f, char at end is = 0x%X\n", d, *end);

          // kann z.B. Auftreten, wenn ungültige Zeichen
          // oder zwei "nicht whitespace" Trennzeichen, z.B. 5;;6
          // In Octave würde ich daraus NA machen
          if (end == start) 
            printf ("no conversion performed\n");
          else
            {
              if (c >= mat.columns ())
                mat.resize (mat.rows(), c + 1, empty_val);
              mat (current_row_idx, c) = d;
            }

          c++;
          if (*end == 0x0A || *end == 0x0D)
            {
              printf ("newline\n");

              current_row_idx++;
              if (mat.rows () < current_row_idx + 1)
                {
                  mat.resize (mat.rows () * GROWTH_FACTOR, mat.columns (), empty_val);
                  printf ("rows after resize = %i\n", mat.rows ());
                }
              c = 0;
            }

          start = end + 1;
        }
      }
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

  int scalar;
  
  Matrix mat;
  octave_idx_type current_row_idx;

  char *buf;
  char *head;

  DECLARE_OV_TYPEID_FUNCTIONS_AND_DATA
};

void load_gz::print (std::ostream& os, bool pr_as_read_syntax)
{
  os << "class load_gz:\n";
  os << "\n  scalar  = " << scalar;
  os << "\n  gz_fn   = '" << gz_fn.c_str () << "'";
  os << "\n  gz_fid  = " << gz_fid;
  os << "\n  rows  = " << rows();
  os << "\n  columns  = " << columns();
  newline (os);
}

DEFMETHOD_DLD (load_gz, interp, args, ,
               "mat = load_gz (fn)\n\
\n\
Reads matrix from VAL.")
{
  static bool type_loaded = false;

  if (! type_loaded)
    {
      load_gz::register_type ();
      interp.mlock ();

      octave_stdout << "installing load_gz at type-id = "
                    << load_gz::static_type_id () << "\n";

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
DEFUN_DLD (mget, args, ,
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
