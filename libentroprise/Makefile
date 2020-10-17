CXX = clang++
LIBNAME = entroprise
HLDIR = Heap-Layers/
HLLDIR = cpp-HyperLogLog/include/
INCLUDES = -I$(HLDIR) -I$(HLDIR)utility -I../include -I$(HLLDIR)

ENTROPRISE_DEBUG = -DENTROPRISE_DEBUG -DENTROPRISE_BACKTRACE
CXXFLAGS = -std=c++17 -flto -g -ffast-math -O2 -fno-builtin-malloc -fvisibility=hidden
ENTROPRISE_FLAGS = $(CXXFLAGS) -o $(LIBNAME)
LIBENTROPRISE_FLAGS = $(CXXFLAGS) -D'CUSTOM_PREFIX(x)=xx\#\#x' -fno-builtin-malloc -pipe -fPIC -D_REENTRANT=1 -shared -Bsymbolic -o lib$(LIBNAME).so -ldl -lpthread -Wl,--no-as-needed

ENTROPRISE_SRC = $(LIBNAME).cpp
LIBENTROPRISE_SRC = lib$(LIBNAME).cpp $(HLDIR)wrappers/gnuwrapper.cpp

all: Heap-Layers entroprise libentroprise

entroprise:
	$(CXX) $(ENTROPRISE_FLAGS) $(INCLUDES) $(ENTROPRISE_SRC)

libentroprise:
	$(CXX) $(LIBENTROPRISE_FLAGS) $(INCLUDES) $(LIBENTROPRISE_SRC)

Heap-Layers:
	git clone https://github.com/emeryberger/Heap-Layers

clean:
	rm -f $(LIBNAME) lib$(LIBNAME).so
