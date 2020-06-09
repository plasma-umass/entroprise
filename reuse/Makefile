CPPFLAGS = -std=c++17 -flto -g -ffast-math -O2 -fno-builtin-malloc -fvisibility=hidden
CXX = clang++
LIBNAME = reuse
HLDIR = /home/msteranka/Heap-Layers
INCLUDES = -I$(HLDIR) -I$(HLDIR)/utility -I../include

LINUX_SRC = lib$(LIBNAME).cpp $(HLDIR)/wrappers/gnuwrapper.cpp
LINUX_COMPILE = $(CXX) $(CPPFLAGS) -D'CUSTOM_PREFIX(x)=xx\#\#x' -fno-builtin-malloc -pipe -fPIC $(INCLUDES) -D_REENTRANT=1 -shared $(LINUX_SRC) -Bsymbolic -o lib$(LIBNAME).so -ldl -lpthread -Wl,--no-as-needed

all: $(LINUX_SRC)
	$(LINUX_COMPILE)

Heap-Layers:
	git clone https://github.com/emeryberger/Heap-Layers

reuse:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o reuse reuse.cpp
