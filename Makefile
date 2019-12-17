.PHONY: all check fuzz style clean

TARGETS=load_gz.oct inc_write testit

all: $(TARGETS)

load_gz.oct: load_gz.cc
	mkoctfile -g $^

# Testprog to create growing .gz files
inc_write: inc_write.c
	$(CC) -Wall -Wextra $^ -o $@ -lz

check: load_gz.oct inc_write
	octave --no-gui --eval 'autoload ("mget", which ("load_gz.oct")); test load_gz'
	octave --no-gui inc_read.m

testit: main.c parse_csv.c
	#gcc -Wall -Wextra -fsanitize=address -static-libasan -O0 $^ -o $@
	gcc -Wall -Wextra -O2 $^ -o $@

# TODO: add fuzzing:
# https://fuzzing-project.org/tutorial1.html
# http://caca.zoy.org/wiki/zzuf/tutorial1
fuzz: testit
	zzuf -s 5:10 -r 0.05 -i ./testit 1>/dev/zero < sample

style:
	astyle --style=gnu -s2 -n *.cc *.c

clean:
	rm -f *.o $(TARGETS) increasing_*.gz octave-workspace
