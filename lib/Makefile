CC      := gcc

QUANTOR := quantor-3.2

# Available SAT Solvers
PICOSAT := picosat-965/libipasir_picosat.so
RISS    := riss_505/libipasirriss_505.so riss_505/libcoprocessor.so.3

# Choose one of them
SATSOLVER := picosat

# Derived Values
SATSOLVER_LIB := $($(shell echo $(SATSOLVER) | tr '[a-z]' '[A-Z]'))
SUBDIRS       := $(QUANTOR) $(dir $(firstword $(PICOSAT))) $(dir $(firstword $(RISS)))

.PHONY: all clean clobber clean_local

all: libquantor.a libipasir_dummy.so libipasir.so

.PRECIOUS: %.tar.gz

#############################################################################
# Quantor
libquantor.a: $(QUANTOR)/libquantor.a
	ln -sf $< .

%/libquantor.a: %.tar.gz
	tar xzf $< && cd $* && \
	patch -p1 < ../quantor-3.2_ipasir.patch && \
	./configure && $(MAKE) $(notdir $@)

quantor-%.tar.gz:
	wget http://fmv.jku.at/quantor/$@

#############################################################################
# SAT Solvers

libipasir_dummy.so: ipasir/libipasir_dummy.so
	ln -sf $< .

libipasir.so: $(SATSOLVER_LIB)
	for i in $^; do ln -sf $$i .; done; rm -f $@; ln -s $< $@

# PicoSat
%/libipasir_picosat.so: %.tar.gz
	tar xzf $< && cd $* && \
	patch -p1 < ../picosat-965_ipasir.patch && \
	./configure.sh -shared && $(MAKE) $(notdir $@)

picosat-%.tar.gz:
	wget http://fmv.jku.at/picosat/$@

# Riss
%/libipasirriss_505.so: %.tar.gz
	tar xzf $< && cd $* && \
	$(MAKE) shared

#############################################################################
clean: clean_local
	for i in $(SUBDIRS); do if [ -d $$i ]; then $(MAKE) -C $$i clean; fi; done

clobber: clean_local
	rm -rf $(SUBDIRS)

clean_local:
	rm -rf lib*.a lib*.so*
