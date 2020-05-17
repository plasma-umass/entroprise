CPPFLAGS = -std=c++17 -flto -g -ffast-math -fno-builtin-malloc -DNDEBUG -fvisibility=hidden
CXX = clang++
LIBNAME = entroprise
INCLUDES = -IHeap-Layers -IHeap-Layers/utility -Icpp-HyperLogLog/include

LINUX_SRC = lib$(LIBNAME).cpp Heap-Layers/wrappers/gnuwrapper.cpp
LINUX_COMPILE = $(CXX) $(CPPFLAGS) -D'CUSTOM_PREFIX(x)=xx\#\#x' -fno-builtin-malloc -pipe -fPIC $(INCLUDES) -D_REENTRANT=1 -shared $(LINUX_SRC) -Bsymbolic -o lib$(LIBNAME).so -ldl -lpthread -Wl,--no-as-needed

all: Heap-Layers $(LINUX_SRC)
	$(LINUX_COMPILE)

test:
	$(CXX) -o test -pthread -std=c++17 test.cpp

Heap-Layers:
	git clone https://github.com/emeryberger/Heap-Layers

HyperLogLog:
	git clone https://github.com/hideo55/cpp-HyperLogLog
