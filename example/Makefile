CXXFLAGS += -std=c++11 -I..
LDFLAGS += -L..
LIBS = -lco

.PHONY:all clean
all: example

example: example.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f example