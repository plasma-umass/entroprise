HLDIR=Heap-Layers/utility/
HPPDIR=cpp-HyperLogLog/include/
CXX=clang++
CXXFLAGS=-shared -Wl,--no-as-needed -ldl -fPIC -std=c++14 -I$(HLDIR) -I$(HPPDIR)
OBJ=libentroprise.o

%.o: %.cpp
	$(CXX) -o $@ $< $(CXXFLAGS) 

libentroprise: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@.so $@.cpp

.PHONY: clean

clean:
	rm $(OBJ)
