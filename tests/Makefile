.PHONY: all check style clean

TARGETS = rand_5e5x8.csv parse inc_write parse.fuzz
OCT = octave -q --no-gui

#CFLAGS = -Wall -Wextra -I ../ -O3
#CFLAGS = -Wall -Wextra -I ../ -O3 -DDEBUG
CFLAGS = -Wall -Wextra -fsanitize=address -I../ -O0

all: $(TARGETS)

rand_5e5x8.csv:
	$(OCT) --eval "a = rand (5e5, 8); save (\"-ascii\", \"rand_5e5x8.csv\", \"a\");"

parse: parse.c ../parse_csv.c
	$(CC) $(CFLAGS) $^ -o $@

# Testprog to create growing .gz files
inc_write: inc_write.c
	$(CC) -Wall -Wextra -O3 $^ -o $@ -lz

check: sample1.csv parse inc_write
	$(OCT) --eval 'addpath ("../"); test load_gz'
	./parse < sample1.csv > sample1.out
	./parse < sample2.csv > sample2.out
	md5sum -c md5sums.ref

benchmark: inc_write
	$(OCT) inc_read.m

sweep_buffer: parse
	./sweep_buffer.sh

## spezial builds:
parse.fuzz: parse.c ../parse_csv.c
	$(CC) -Wall -Wextra -I ../ -O3 $^ -o $@

# fuzzing:
# https://fuzzing-project.org/tutorial1.html
# http://caca.zoy.org/wiki/zzuf/tutorial1
fuzz: parse.fuzz
	zzuf -q -s 0:5000 -r 0.05 ./parse.fuzz sample1.csv
	zzuf -q -s 0:5000 -r 0.05 ./parse.fuzz sample2.csv

style:
	astyle --style=gnu -s2 -n *.c

clean:
	rm -f $(TARGETS) *.out octave-workspace

