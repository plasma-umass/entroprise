CXX=clang++
CXXFLAGS=-IHeap-Layers -pthread -std=c++14
OBJ=entroprise.o

all: $(OBJ) Heap-Layers
	$(CXX) $(CXXFLAGS) -o entroprise $(OBJ)

Heap-Layers:
	git clone https://github.com/emeryberger/Heap-Layers

%.o: %.c $(DEPS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm -f *.o
