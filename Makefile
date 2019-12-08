.PHONY: all run style clean

TARGETS=load_gz.oct

all: $(TARGETS)

load_gz.oct: load_gz.cc
	mkoctfile $^

run: all
	octave --no-gui --eval 'autoload ("mget", which ("load_gz.oct")); x=load_gz("bar"), mget(x), x'

style:
	astyle --style=gnu -s2 -n *.cc

clean:
	rm -f *.o $(TARGETS)
