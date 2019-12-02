CXX=clang++
CXXFLAGS=-std=c++11 -I ./
DEPS=runs.h ks.h
OBJ=malloc.o ks.o runs.o

%.o: %.c $(DEPS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(OBJ)
	$(CXX) -std=c++11 -o malloc $(OBJ)

.PHONY: clean

clean:
	rm -f *.o
