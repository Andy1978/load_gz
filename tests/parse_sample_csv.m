#!/usr/bin/env -S octave -q

fn = "sample.csv";

addpath ("../");
autoload ("mget", "load_gz.oct");
autoload ("get", "load_gz.oct");

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
assert (s.rows, 7)
assert (s.columns, 3)
assert (s.comments{1}, "# testing mixed delimiter and linebreaks")
assert (s.comments{2}, "#")
assert (s.comments{3}, "# CR linebreak (classic apple)")
assert (s.comments{4}, "# CR+LF linebreak (windoze)")
assert (m, ref);
