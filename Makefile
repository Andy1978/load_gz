.PHONY: all check fuzz style clean

TARGETS = load_gz.oct

all: $(TARGETS)

load_gz.oct: load_gz.cc parse_csv.c
	mkoctfile -Wall -Wextra -g $^

check: load_gz.oct
	$(MAKE) -C tests $@

style:
	astyle --style=gnu -s2 -n *.cc *.c

clean:
	rm -f *.o $(TARGETS) increasing_*.gz octave-workspace
	$(MAKE) -C tests $@
