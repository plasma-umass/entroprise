CXX = clang++
LIBNAME = reuse
HLDIR = Heap-Layers/
INCLUDES = -I$(HLDIR) -I$(HLDIR)utility -I../include

CXXFLAGS = -std=c++17 -flto -g -ffast-math -O2 -fno-builtin-malloc -fvisibility=hidden
REUSE_FLAGS = $(CXXFLAGS) -o $(LIBNAME)
LIBREUSE_FLAGS = $(CXXFLAGS) -D'CUSTOM_PREFIX(x)=xx\#\#x' -fno-builtin-malloc -pipe -fPIC -D_REENTRANT=1 -shared -Bsymbolic -o lib$(LIBNAME).so -ldl -lpthread -Wl,--no-as-needed

REUSE_SRC = $(LIBNAME).cpp
LIBREUSE_SRC = lib$(LIBNAME).cpp $(HLDIR)wrappers/gnuwrapper.cpp

all: Heap-Layers reuse libreuse

reuse:
	$(CXX) $(REUSE_FLAGS) $(INCLUDES) $(REUSE_SRC)

libreuse:
	$(CXX) $(LIBREUSE_FLAGS) $(INCLUDES) $(LIB_REUSE_SRC)

Heap-Layers:
	git clone https://github.com/emeryberger/Heap-Layers

clean:
	rm -f $(LIBNAME) lib$(LIBNAME).so
