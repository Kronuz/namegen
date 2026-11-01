.POSIX:
.SUFFIXES: .cc
CXX      = c++
CXXFLAGS = -std=c++11 -Wall -Wextra -O3 -g3 -I.

all: namegen

namegen: namegen.o examples/demo.o
	$(CXX) $(LDFLAGS) -o $@ namegen.o examples/demo.o $(LDLIBS)

namegen.o: namegen.cc namegen.h
examples/demo.o: examples/demo.cc namegen.h

clean:
	rm -rf namegen namegen.o examples/demo.o

.cc.o:
	$(CXX) -c $(CXXFLAGS) -o $@ $<
