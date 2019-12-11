#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <zlib.h>

#define BLOCK_LINES 2000
#define DELAY_MS 1000

static volatile char do_exit = 0;

void termination_handler (int signum)
{
  //fprintf (stderr, "Received signal %i, terminating program\n", signum);
  (void) signum;
  do_exit = 1;
}

int main (int argc, char *argv[])
{
  if (argc != 2)
    {
      fprintf (stderr, "\n USAGE: %s FILE.GZ\n", argv[0]);
      fprintf (stderr, " Creates an increasing (%.1f lines per second) gzipped file FILE.GZ\n\n", BLOCK_LINES * 1e3 / DELAY_MS);
      return (-1);
    }

  gzFile foo = gzopen (argv[1], "wb");
  if (! foo)
    {
      fprintf (stderr, "ERROR: Opening '%s' failed with error code %i (%s)\n", argv[0], errno, strerror (errno));
      return -1;
    }

  if (signal (SIGINT, termination_handler) == SIG_IGN)
    signal (SIGINT, SIG_IGN);

  if (signal (SIGTERM, termination_handler) == SIG_IGN)
    signal (SIGTERM, SIG_IGN);

  if (signal (SIGPIPE, termination_handler) == SIG_IGN)
    signal (SIGPIPE, SIG_IGN);

  while (! do_exit)
    {
      for (int k = 0; k < BLOCK_LINES; ++k)
        {
          double d = rand () / (RAND_MAX - 1.0) * 100;
          gzprintf (foo, "%f %f %f %f\n", d, 2*d, 3*d, 4*d);
        }
      gzflush (foo, Z_SYNC_FLUSH);
      //gzflush (foo, Z_FINISH);
      usleep (DELAY_MS * 1000);
    }
  
  gzclose (foo);
  return 0;

}
