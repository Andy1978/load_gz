.PHONY: clean

TARGETS=load_gz.oct

all: $(TARGETS)

load_gz.oct: load_gz.cc
	mkoctfile $^

run: all
	octave --no-gui --eval 'autoload ("mget", which ("load_gz.oct")); x=load_gz("bar"), mget(x)'

clean:
	rm -f *.o $(TARGETS)
