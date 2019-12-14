fn = tempname();

fid = fopen (fn, "wb");

fprintf (fid, "# testing mixed delimiter and linebreaks\n");
fprintf (fid, "#\n");
fprintf (fid, "7 8.12 9.333\n");
#fprintf (fid, "1;2.3,4.5\n");
#fprintf (fid, "\n");
#fprintf (fid, "2\t3.4\t5.6\n");
#fprintf (fid, "# CR linebreak (classic apple)\r");
#fprintf (fid, "10 20 30\r");
#fprintf (fid, "15 25 35.6\r");
#fprintf (fid, "# CR+LF linebreak (windoze)\r\n");
#fprintf (fid, "2.1255363456 3.123467384456 4.874443367876\r\n");
#fprintf (fid, "\r\n");
#fprintf (fid, "3.14156 2.718 40\r\n");
fclose (fid);

autoload ("mget", which ("load_gz.oct"));
autoload ("get", which ("load_gz.oct"));
x = load_gz (fn);
m = mget (x)

ref = [7 8.12 9.333;
       1 2.3 4.5;
       2 3.4 5.6;
       10 20 30;
       15 25 35.6;
       2.1255363456 3.123467384456 4.874443367876;
       3.14156 2.718 40];
       
s = get (x)
assert (s.fn, fn)
#assert (s.rows, 7)
assert (s.columns, 3)
assert (s.comments{1}, "# testing mixed delimiter and linebreaks")
assert (s.comments{2}, "#")
#assert (s.comments{3}, "# CR linebreak (classic apple)")
#assert (s.comments{4}, "# CR+LF linebreak (windoze)")

#assert (m, ref);

unlink (fn)
