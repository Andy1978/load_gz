#!/usr/bin/env -S octave -q

## create CSV
fn = "sample.csv";

fid = fopen (fn, "wb");

fprintf (fid, "# testing mixed delimiter and linebreaks\n");
fprintf (fid, "#\n");
fprintf (fid, "7 8.12 9.333\n");
fprintf (fid, "1;2.3,4.5\n");
fprintf (fid, "\n");
fprintf (fid, "2\t3.4\t5.6\n");
fprintf (fid, "# CR linebreak (classic apple)\r");
fprintf (fid, "10 20 30\r");
fprintf (fid, "15 25 35.6\r");
fprintf (fid, "# CR+LF linebreak (windoze)\r\n");
fprintf (fid, "2.1255363456 3.123467384456 4.874443367876\r\n");
fprintf (fid, "\r\n");
fprintf (fid, "3.14156 2.718 40\r\n");
fclose (fid);
printf ("created '%s'\n", fn);
