CXXFLAGS = -std=c++17 -flto -g -ffast-math -O2 -fno-builtin-malloc -O2 -DNRANDOMNESS_DEBUG -fvisibility=hidden
CXX = clang++
LIBNAME = randomness
HLDIR = /home/msteranka/Heap-Layers
INCLUDES = -I$(HLDIR) -I$(HLDIR)/wrappers -I$(HLDIR)/utility -I../include

LINUX_SRC = lib$(LIBNAME).cpp $(HLDIR)/wrappers/gnuwrapper.cpp
LINUX_COMPILE = $(CXX) $(CXXFLAGS) -D'CUSTOM_PREFIX(x)=xx\#\#x' -I/usr/include/nptl -fno-builtin-malloc -pipe -fPIC $(INCLUDES) -D_REENTRANT=1 -shared $(LINUX_SRC) -Bsymbolic -o lib$(LIBNAME).so -ldl -lpthread

all: $(LINUX_SRC) randomness
	$(LINUX_COMPILE)

randomness: randomness.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o randomness randomness.cpp
