CXXFLAGS := -std=gnu++11 -Wall -O3 -I../../lib/quantor-3.2
CXX	 := g++
CC	 := g++

OBJECTS  := Component.o CompDecl.o Statement.o PortDecl.o WireDecl.o Expression.o ParamDecl.o Lib.o Root.o Context.o Result.o
LIB_DIR  := ../../lib
LIBS     := $(patsubst %,$(LIB_DIR)/lib%.a, quantor satsolver)

.PHONY: default all clean clobber

default: libqbm.a
all: default test

libqbm.a: $(OBJECTS)
	$(AR) rc $@ $^

$(OBJECTS): $(LIBS)

test: LDLIBS  += -lpthread
test: test.o libqbm.a $(LIBS)

$(LIBS):
	$(MAKE) -C $(dir $@) $(notdir $@)

clean:
	rm -rf *~ lib*.a *.o test

clobber: clean
