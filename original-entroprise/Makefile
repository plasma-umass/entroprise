CXX=clang++
CXXFLAGS=-IHeap-Layers -pthread -std=c++14
OBJ=entroprise.o

%.o: %.c $(DEPS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(OBJ)
	$(CXX) $(CXXFLAGS) -o entroprise $(OBJ)

.PHONY: clean

clean:
	rm -f *.o
