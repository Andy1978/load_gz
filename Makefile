.PHONY: clean

CXX = g++

CXXFLAGS = -Wall -Wextra -O3

LDFLAGS = -lz

ARCH = $(shell uname -m)
ifeq ($(ARCH),x86_64)
	CXXFLAGS := -fsanitize=address -fsanitize=leak $(CXXFLAGS)
endif

TARGETS=test

all: $(TARGETS)

test:	test.cc zfstream.cc
	$(CXX) $(CXXFLAGS) -Wall -Wextra $^ $(LDFLAGS) -o $@

clean:
	rm -f *.o $(TARGETS)
