## test increasing .gz file
autoload ("mget", which ("load_gz.oct"))

r = rand (40000, 2);

# FIXME: This doesn't work for /tmp/? WTF?
fn1 = "increasing_in.gz";
fn2 = "increasing_out.gz";
save ("-z", "-ascii", fn1, "r")
save ("-z", "-ascii", fn2, "r")
x = load_gz (fn2);
for k = 1:6
 tic; ref = load (fn2); t_load = toc();
 tic; ; m = mget (x); t_load_gz = toc();
 printf ("%10i speed up is %.1f\n", rows (m), t_load/t_load_gz);
 assert (m, ref);
 cmd = sprintf ('cat "%s" "%s" | sponge "%s"', fn1, fn2, fn2);
 system (cmd);
endfor
unlink (fn1);
unlink (fn2);
