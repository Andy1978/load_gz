.PHONY: all check style clean

TARGETS = rand_5e5x8.csv parse_stdin inc_write
OCT = octave -q --no-gui

#CFLAGS = -Wall -Wextra -I ../ -O3
#CFLAGS = -Wall -Wextra -I ../ -O3 -DDEBUG
CFLAGS = -Wall -Wextra -fsanitize=address -static-libasan -I../ -O0

all: $(TARGETS)

rand_5e5x8.csv:
	$(OCT) --eval "a = rand (5e5, 8); save (\"-ascii\", \"rand_5e5x8.csv\", \"a\");"

parse_stdin: parse_stdin.c ../parse_csv.c
	$(CC) $(CFLAGS) $^ -o $@

# Testprog to create growing .gz files
inc_write: inc_write.c
	$(CC) -Wall -Wextra -O3 $^ -o $@ -lz

check: sample.csv parse_stdin inc_write
	$(OCT) --eval 'addpath ("../"); test load_gz'
	#./parse_stdin < sample.csv
	#$(OCT) parse_sample_csv.m

benchmark: inc_write
	$(OCT) inc_read.m


## spezial builds:

# fuzzing:
# https://fuzzing-project.org/tutorial1.html
# http://caca.zoy.org/wiki/zzuf/tutorial1
fuzz: parse_stdin
	zzuf -q -s 0:10000 -r 0.05 ./parse_stdin sample1.csv

style:
	astyle --style=gnu -s2 -n *.c

clean:
	rm -f $(TARGETS) *.out octave-workspace

