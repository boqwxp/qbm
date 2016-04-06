all: libipasir_dummy.so

ipasir_dummy.o: CFLAGS += -Wall -Wno-unused-parameter -fPIC
libipasir_dummy.so: ipasir_dummy.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,libipasir.so -o$@ $<
