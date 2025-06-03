.PHONY: all style clean

TARGETS = load_gz.oct load_xz.oct peakdet.oct find_first.oct

all: $(TARGETS)

load_gz.oct: load_gz.cc parse_csv.c
	mkoctfile -Wall -Wextra -g -lz $^

load_xz.oct: load_xz.cc parse_csv.c
	#mkoctfile -Wall -Wextra -DDEBUG -g -llzma $^
	mkoctfile -Wall -Wextra -g -llzma $^

run: load_xz.oct
	octave --eval 'x = load_xz ("foo.xz"); mget (x), mget (x)'

%.oct: %.cc
	mkoctfile -Wall -Wextra $^

style:
	astyle --style=gnu -s2 -n *.cc *.c

clean:
	rm -f *.o $(TARGETS) octave-workspace
