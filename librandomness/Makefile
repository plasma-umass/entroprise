CXXFLAGS = -std=c++17 -flto -g -ffast-math -fno-builtin-malloc -O2 -DNDEBUG -fvisibility=hidden
CXX = clang++
LIBNAME = randomness
HLDIR = /home/msteranka/Heap-Layers
RANDDIR = /home/msteranka/entroprise/randomness-standalone
INCLUDES = -I. -I$(HLDIR) -I$(HLDIR)/wrappers -I$(HLDIR)/utility -I$(RANDDIR)

LINUX_SRC = lib$(LIBNAME).cpp $(HLDIR)/wrappers/gnuwrapper.cpp
LINUX_COMPILE = $(CXX) $(CXXFLAGS) -D'CUSTOM_PREFIX(x)=xx\#\#x' -I/usr/include/nptl -fno-builtin-malloc -pipe -fPIC $(INCLUDES) -D_REENTRANT=1 -shared $(LINUX_SRC) -Bsymbolic -o lib$(LIBNAME).so -ldl -lpthread

all: $(LINUX_SRC) ks
	$(LINUX_COMPILE)

ks: ks.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o ks ks.cpp
