fn = "sample1.csv";

addpath ("../");

x = load_gz (fn);
m = mget (x)

ref = [7 8.12 9.333 NA;
       1 2.3 4.5 NA;
       2 3.4 5.6 NA;
       10 20 30 NA;
       15 25 35.6 NA;
       2.1255363456 3.123467384456 4.874443367876 NA;
       3.14156 2.718 40 NA;
       NA 5.45 6.78 9e3;
       41200 90 45054 NA;
       16.328125 0.000032 2.718 NA];
       
s = get (x);
assert (s.fn, fn)
assert (s.rows, 10)
assert (s.columns, 4)
assert (s.comments{1}, "# testing mixed delimiter and linebreaks")
assert (s.comments{2}, "#")
assert (s.comments{3}, "# CR linebreak (classic apple)")
assert (s.comments{4}, "# CR+LF linebreak (windoze)")
assert (m, ref);
