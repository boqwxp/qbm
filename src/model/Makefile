CXXFLAGS := -std=gnu++11 -Wall -O3 -I../../lib/quantor-3.2
CXX	 := g++
CC	 := g++

OBJECTS  := Component.o CompDecl.o Statement.o PortDecl.o WireDecl.o Expression.o ParamDecl.o Lib.o Root.o Context.o Result.o
LIB_DIR  := ../../lib
LIBS     := $(LIB_DIR)/libquantor.a $(LIB_DIR)/libipasir_dummy.so

.PHONY: default all clean clobber

default: libqbm.a
all: default test

libqbm.a: $(OBJECTS)
	$(AR) rcs $@ $^

$(OBJECTS): $(LIBS)

test: test.o libqbm.a $(LIBS)

$(LIBS):
	$(MAKE) -C $(dir $@) $(notdir $@)

clean:
	rm -rf *~ lib*.a *.o test

clobber: clean
