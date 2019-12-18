.PHONY: all check style clean

TARGETS = sample.csv parse_stdin inc_write

all: $(TARGETS)

sample.csv: create_sample_csv.m
	./$<

parse_stdin: parse_stdin.c ../parse_csv.c
	$(CC) -Wall -Wextra -fsanitize=address -I../ -O0 $^ -o $@
	#$(CC) -Wall -Wextra -I../ -O0 -ggdb $^ -o $@
	#$(CC) -Wall -Wextra -I../ -O2 $^ -o $@

# Testprog to create growing .gz files
inc_write: inc_write.c
	$(CC) -Wall -Wextra $^ -o $@ -lz

check: sample.csv
	./parse_stdin < sample.csv
	#octave --no-gui inc_read.m
	#./parse_sample_csv.m

# TODO: add fuzzing:
# https://fuzzing-project.org/tutorial1.html
# http://caca.zoy.org/wiki/zzuf/tutorial1
fuzz: parse_stdin
	zzuf -s 5:10 -r 0.05 -i ./parse_stdin 1>/dev/zero < sample

style:
	astyle --style=gnu -s2 -n *.cc *.c

clean:
	rm -f $(TARGETS) octave-workspace
