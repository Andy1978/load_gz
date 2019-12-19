.PHONY: all check style clean

TARGETS = sample.csv parse_stdin inc_write
OCT = octave -q --no-gui
all: $(TARGETS)

sample.csv: create_sample_csv.m
	$(OCT) $<

parse_stdin: parse_stdin.c ../parse_csv.c
	$(CC) -Wall -Wextra -fsanitize=address -I../ -O0 $^ -o $@
	#$(CC) -Wall -Wextra -I../ -O0 -ggdb $^ -o $@
	#$(CC) -Wall -Wextra -I../ -O2 $^ -o $@

# Testprog to create growing .gz files
inc_write: inc_write.c
	$(CC) -Wall -Wextra $^ -o $@ -lz

check: sample.csv parse_stdin inc_write
	$(OCT) --eval 'addpath ("../"); test load_gz'
	#./parse_stdin < sample.csv
	#$(OCT) parse_sample_csv.m

benchmark: inc_write
	$(OCT) inc_read.m

# fuzzing:
# https://fuzzing-project.org/tutorial1.html
# http://caca.zoy.org/wiki/zzuf/tutorial1
fuzz: parse_stdin
	zzuf -q -s 0:10000 -r 0.05 ./parse_stdin sample.csv

style:
	astyle --style=gnu -s2 -n *.c

clean:
	rm -f $(TARGETS) octave-workspace

