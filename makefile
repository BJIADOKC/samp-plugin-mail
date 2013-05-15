GPP = g++
GCC = gcc

OUTFILE = "compiled/mail.so"

COMPILE_FLAGS = -c -m32 -O3 -fPIC -w -DLINUX -Wall -I./sdk/amx/

all:
	$(GCC) $(COMPILE_FLAGS) ./sdk/amx/*.c
	$(GPP) $(COMPILE_FLAGS) ./sdk/*.cpp
	$(GPP) $(COMPILE_FLAGS) ./jwsmtp/*.cpp
	$(GPP) $(COMPILE_FLAGS) *.cpp
	$(GPP) -O3 -fshort-wchar -shared -o $(OUTFILE) *.o -lpthread
	rm -f *.o
