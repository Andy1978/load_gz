.PHONY: all check style clean

TARGETS=load_gz.oct

all: $(TARGETS)

load_gz.oct: load_gz.cc
	mkoctfile $^

check: load_gz.oct
	octave --no-gui --eval 'autoload ("mget", which ("load_gz.oct")); test load_gz'
	octave --no-gui benchmark_inc.m

style:
	astyle --style=gnu -s2 -n *.cc

clean:
	rm -f *.o $(TARGETS) increasing_*.gz
