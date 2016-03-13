.PHONY: all clean

all: test

test: test.o Component.o CompDecl.o Statement.o PortDecl.o WireDecl.o Expression.o ParamDecl.o Lib.o Root.o
	g++ -std=gnu++11 -Wall -O3 -otest $^

%.o: %.cpp
	g++ -std=gnu++11 -Wall -O3 -c -o$@ $^

clean:
	rm -rf *~ *.o
