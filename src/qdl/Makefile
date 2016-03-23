CXXFLAGS := -std=gnu++11 -Wall -O3 -I../model -I../../lib/quantor-3.2
CXX      := g++
CC       := g++
LIB_DIR  := ../../lib

.PHONY: all clean clobber

all: qdlsolve

qdlsolve: LDLIBS += -lpthread
qdlsolve: qdlsolve.o QdlParser.o ../model/libqbm.a $(patsubst %,$(LIB_DIR)/lib%.a, quantor satsolver)

QdlParser.cpp QdlParser.hpp: QdlParser.ypp
	wisent $<

../model/libqbm.a $(LIB_DIR)/lib%.a:
	$(MAKE) -C $(dir $@) $(notdir $@)

clobber: clean
	rm -rf QdlParser.cpp QdlParser.hpp
clean:
	rm -rf *~ *.o
