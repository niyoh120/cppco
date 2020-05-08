CXXFLAGS += -std=c++11

.PHONY: all clean example

all: libco.a example

lib: libco.a

libco.a: co.o
	ar rcs $@ $^

example: lib
	make -C example

CLEANEXTS = o a 
clean:
	for file in $(CLEANEXTS); do rm -f *.$$file; done
	-make -C example clean

co.o: co.hpp