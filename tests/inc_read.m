## Starts ./inc_write which creates a growing .gz file, synced with
## Z_SYNC_FLUSH so that the decompressor is able to decode it

addpath ("../")

#fn = "foobar.gz";
fn = tempname ();
unlink (fn);
system (sprintf ('./inc_write "%s"', fn), [], "async");
pause (0.1);

x = load_gz (fn)

assert_cnt = 0;
for k=1:40
  # load from core
  tic;
  ref = load (fn);
  t_load = toc();
  printf ("load from core :%8i rows, t_load =    %6.1fms\n", rows(ref), t_load * 1e3);

  # incremental load_gz
  tic;
  y = mget (x);
  t_load_gz = toc ();
  printf ("load_gz        :%8i rows, t_load_gz = %6.1fms\n", rows(y), t_load_gz * 1e3);

  if (rows(ref) == rows(y)) # This might not always be true due to async write glitches
    assert (ref, y);
    assert_cnt++;
  endif

  fflush (stdout);
  pause (0.2)
endfor

assert (assert_cnt > 20);

clear x
system ("killall inc_write");
unlink (fn);
