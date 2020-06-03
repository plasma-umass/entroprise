CPPFLAGS = -std=c++17 -flto -g -ffast-math -O2 -fno-builtin-malloc -DNENTROPRISE_DEBUG -fvisibility=hidden
CXX = clang++
LIBNAME = entroprise
HLDIR = /home/msteranka/Heap-Layers
INCLUDES = -I$(HLDIR) -I$(HLDIR)/utility -Icpp-HyperLogLog/include -I../include

LINUX_SRC = lib$(LIBNAME).cpp $(HLDIR)/wrappers/gnuwrapper.cpp
LINUX_COMPILE = $(CXX) $(CPPFLAGS) -D'CUSTOM_PREFIX(x)=xx\#\#x' -fno-builtin-malloc -pipe -fPIC $(INCLUDES) -D_REENTRANT=1 -shared $(LINUX_SRC) -Bsymbolic -o lib$(LIBNAME).so -ldl -lpthread -Wl,--no-as-needed

all: $(LINUX_SRC) entroprise.cpp
	$(LINUX_COMPILE)

entroprise:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o entroprise entroprise.cpp

test:
	$(CXX) -o test -pthread -std=c++17 test.cpp

Heap-Layers:
	git clone https://github.com/emeryberger/Heap-Layers

HyperLogLog:
	git clone https://github.com/hideo55/cpp-HyperLogLog
