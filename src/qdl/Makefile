CXXFLAGS := -std=gnu++11 -Wall -O3 -I../model -I../../lib/quantor-3.2
CXX      := g++
CC       := g++

OBJECTS  := qdlsolve.o QdlParser.o
LIB_DIR  := ../../lib
LIBS     := $(LIB_DIR)/libquantor.a $(LIB_DIR)/libipasir_dummy.so

.PHONY: all clean clobber

all: qdlsolve

qdlsolve: LDLIBS += -Wl,-rpath,'$$ORIGIN/../lib'
qdlsolve: $(OBJECTS) ../model/libqbm.a $(LIBS)

$(OBJECTS): $(LIBS)

QdlParser.cpp QdlParser.hpp: QdlParser.ypp
	wisent $<

../model/libqbm.a $(LIBS):
	$(MAKE) -C $(dir $@) $(notdir $@)

clobber: clean
	rm -rf QdlParser.cpp QdlParser.hpp
clean:
	rm -rf *~ *.o
