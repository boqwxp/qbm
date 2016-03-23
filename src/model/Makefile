CXXFLAGS := -std=gnu++11 -Wall -O3 -I../../lib/quantor-3.2
CXX	 := g++
CC	 := g++
LIB_DIR  := ../../lib

.PHONY: default all clean clobber

default: libqbm.a
all: default test

libqbm.a: Component.o CompDecl.o Statement.o PortDecl.o WireDecl.o Expression.o ParamDecl.o Lib.o Root.o Context.o Result.o
	$(AR) rc $@ $^

test: LDLIBS  += -lpthread
test: test.o libqbm.a $(patsubst %,$(LIB_DIR)/lib%.a, quantor satsolver)

$(LIB_DIR)/lib%.a:
	$(MAKE) -C $(dir $@) $(notdir $@)

clean:
	rm -rf *~ lib*.a *.o test

clobber: clean
