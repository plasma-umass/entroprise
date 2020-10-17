CXX = clang++
CXXFLAGS = -std=c++17 -pthread
INCLUDES = -I../include

all: test mt-randomness malloc-randomness

test:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o test test.cpp

malloc-randomness:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o malloc-randomness malloc-randomness.cpp

mt-randomness:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o mt-randomness mt-randomness.cpp

clean:
	rm test mt-randomness malloc-randomness
