CXXFLAGS := -std=gnu++11 -Wall -O3 -I../model -I../../lib/quantor-3.2
CXX      := g++
CC       := g++

OBJECTS  := qdlsolve.o QdlParser.o
LIB_DIR  := ../../lib
LIBS     := $(patsubst %,$(LIB_DIR)/lib%.a, quantor satsolver)

.PHONY: all clean clobber

all: qdlsolve

qdlsolve: LDLIBS += -lpthread
qdlsolve: $(OBJECTS) ../model/libqbm.a $(LIBS)

QdlParser.cpp QdlParser.hpp: QdlParser.ypp
	wisent $<

$(OBJECTS): $(LIBS)

../model/libqbm.a $(LIBS):
	$(MAKE) -C $(dir $@) $(notdir $@)

clobber: clean
	rm -rf QdlParser.cpp QdlParser.hpp
clean:
	rm -rf *~ *.o
