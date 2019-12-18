.PHONY: all check fuzz style clean

TARGETS = load_gz.oct

all: $(TARGETS)

load_gz.oct: load_gz.cc parse_csv.c
	mkoctfile -Wall -Wextra -g $^

./tests/inc_write:
	$(MAKE) -C ./tests

check: load_gz.oct ./tests/inc_write
	octave --no-gui --eval 'autoload ("mget", which ("load_gz.oct")); test load_gz'

style:
	astyle --style=gnu -s2 -n *.cc *.c

clean:
	rm -f *.o $(TARGETS) increasing_*.gz octave-workspace
