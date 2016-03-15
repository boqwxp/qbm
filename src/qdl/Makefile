CXXFLAGS := -std=gnu++11 -Wall -O3 -I../model -I../../lib/quantor-3.2
CXX      := g++
CC       := g++

all: test

test: LDFLAGS += -L../../lib
test: LDLIBS += -lquantor -lpicosat

test: test.o $(filter-out ../model/test.o,$(wildcard ../model/*.o)) QdlParser.o

QdlParser.cpp QdlParser.hpp: QdlParser.ypp
	wisent $<

clobber: clean
	rm -rf QdlParser.cpp QdlParser.hpp
clean:
	rm -rf *~ *.o
