CXX=clang++
CXXFLAGS=-pthread -std=c++11
OBJ=entroprise.o

%.o: %.c $(DEPS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(OBJ)
	$(CXX) $(CXXFLAGS) -o entroprise $(OBJ)

.PHONY: clean

clean:
	rm -f *.o
