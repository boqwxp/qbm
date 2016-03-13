CXXFLAGS := -std=gnu++11 -Wall -O3 -I../../lib/quantor-3.2
CXX	 := g++
CC	 := g++

.PHONY: all clean

all: test

test: LDFLAGS += -L../../lib
test: LDLIBS += -lquantor -lpicosat

test: test.o Component.o CompDecl.o Statement.o PortDecl.o WireDecl.o Expression.o ParamDecl.o Lib.o Root.o Result.o

clean:
	rm -rf *~ *.o test
