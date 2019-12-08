.PHONY: clean

CXX = g++

CXXFLAGS = -Wall -Wextra -O3

LDFLAGS = -lz

ARCH = $(shell uname -m)
ifeq ($(ARCH),x86_64)
	CXXFLAGS := -fsanitize=address -fsanitize=leak $(CXXFLAGS)
endif

TARGETS=test main1

all: $(TARGETS)

test:	test.cc zfstream.cc
	$(CXX) $(CXXFLAGS) -Wall -Wextra $^ $(LDFLAGS) -o $@

main1: main1.c
	$(CC) -Wall -Wextra -fsanitize=address -fsanitize=leak $^ -lz -o $@

load_gz.oct: load_gz.cc
	mkoctfile $^

run: load_gz.oct
	octave --no-gui --eval 'autoload ("mget", which ("load_gz.oct")); x=load_gz("bar"), mget(x)'

clean:
	rm -f *.o $(TARGETS)
