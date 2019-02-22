#include <iostream>
#include <cstring>
#include "zfstream.h"

using namespace std;

int main ()
{
  string gz_filename = "foo.gz";
  gzifstream inf;

  unsigned columns = 7;

  double values[columns];

  inf.open(gz_filename.c_str());
  if (inf.fail())
    {
      cerr << "ERROR open(" << gz_filename << ") error: " << strerror (errno) << endl;
      return -1;
    }

  unsigned row = 0;
  unsigned col = 0;
  while (! inf.eof())
    {
      inf >> values[col++];
      if (col >= columns)
        {
          //add_data (values[channel_ids["Weg"]],
          //          values[channel_ids["Kraft"]],
          //          values[channel_ids["DI1"]],
          //          values[channel_ids["DI2"]]);
          col = 0;
          row++;
          printf ("%f %f\n", values[0], values[columns - 1]);
        }
    }
  inf.close();
  return 0;
}
