.PHONY: all check style clean

TARGETS = rand_5e5x8.csv rand_50x8.csv parse inc_write parse_no_ASAN parse_afl
OCT = octave -q --no-gui

# Use ASAN
CFLAGS = -Wall -Wextra -fsanitize=address -I../ -O0

all:
	@echo "No default target."
	@echo ""
	@echo "Available targets:"
	@echo "  check        : run basic tests with ./parse"
	@echo "  benchmark    : test incremental reads with load_gz.oct"
	@echo "  sweep_buffer : test ./parse with different buffer sizes"
	@echo "  zzuf         : fuzz ./parse with zzuf"
	@echo "  afl          : fuzz ./parse with afl"
	@echo "  style        : run astyle on source"
	@echo "  clean        : you guess it..."

rand_5e5x8.csv:
	$(OCT) --eval "a = rand (5e5, 8); save (\"-ascii\", \"rand_5e5x8.csv\", \"a\");"

rand_50x8.csv:
	$(OCT) --eval "a = rand (50, 8); save (\"-ascii\", \"rand_50x8.csv\", \"a\");"

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

../load_gz.oct:
	$(MAKE) -C ../

benchmark: inc_write ../load_gz.oct
	$(OCT) inc_read.m

sweep_buffer: parse
	./sweep_buffer.sh

# fuzzing:
# https://fuzzing-project.org/tutorial1.html
# http://caca.zoy.org/wiki/zzuf/tutorial1

fuzz: zzuf afl

parse_no_ASAN: parse.c ../parse_csv.c
	$(CC) -Wall -Wextra -I ../ -O2 $^ -o $@

zzuf: parse_no_ASAN
	zzuf -c -q -j $$(nproc) -s 0:20000 -r 0.05 ./parse_no_ASAN -b 43 sample1.csv
	zzuf -c -q -j $$(nproc) -s 0:20000 -r 0.05 ./parse_no_ASAN -b 43 sample2.csv

parse_afl: parse.c ../parse_csv.c
	afl-gcc -Wall -Wextra -I../ -O2 $^ -o $@

afl: parse_afl rand_50x8.csv
	mkdir -p afl_in && cp rand_50x8.csv sample*.csv afl_in && mkdir -p afl_out
	afl-fuzz -i afl_in -o afl_out ./parse_afl @@

style:
	astyle --style=gnu -s2 -n *.c

clean:
	rm -f $(TARGETS) *.out octave-workspace
	rm -rf afl_in

