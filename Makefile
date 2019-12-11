.PHONY: all check style clean

TARGETS=load_gz.oct inc_write

all: $(TARGETS)

load_gz.oct: load_gz.cc
	mkoctfile $^

# Testprog to create growing .gz files
inc_write: inc_write.c
	$(CC) -Wall -Wextra $^ -o $@ -lz

check: load_gz.oct inc_write
	octave --no-gui --eval 'autoload ("mget", which ("load_gz.oct")); test load_gz'
	octave --no-gui inc_read.m

style:
	astyle --style=gnu -s2 -n *.cc *.c

clean:
	rm -f *.o $(TARGETS) increasing_*.gz
