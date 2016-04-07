# Choose one of the SAT Solvers below
SATSOLVER ?= picosat

CC      := gcc
QUANTOR := quantor-3.2

# Available SAT Solvers
PICOSAT     := picosat-965
PICOSATLIBS := $(PICOSAT)/libipasir_picosat.so
RISS        := riss_505
RISSLIBS    := $(RISS)/libipasir$(RISS).so $(RISS)/libcoprocessor.so.3

# Derived Values
SATSOLVERLIBS := $($(shell echo $(SATSOLVER) | tr '[a-z]' '[A-Z]')LIBS)
SUBDIRS       := $(QUANTOR) $(dir $(firstword $(PICOSATLIBS)) $(firstword $(RISSLIBS)))

.PHONY: all clean clobber clean_local

## Standard Targets #########################################################
all: libquantor.a libipasir_dummy.so libipasir.so

clean: clean_local
	for i in $(SUBDIRS); do if [ -d $$i ]; then $(MAKE) -C $$i clean; fi; done

clobber: clean_local
	rm -rf $(SUBDIRS)

clean_local:
	rm -rf lib*.a lib*.so*

# Make sure downloaded archives stay around
.SECONDARY:

#############################################################################
# Quantor
libquantor.a: $(QUANTOR)/libquantor.a
	ln -sf $< .

%/libquantor.a %/libipasir_dummy.so: %/Makefile
	$(MAKE) -C $* $(notdir $@)

%/Makefile: %.tar.gz
	rm -rf $*/ && tar xzf $< && cd $* && \
	patch -p1 < ../quantor-3.2_ipasir.patch && \
	./configure

quantor-%.tar.gz:
	wget http://fmv.jku.at/quantor/$@

#############################################################################
# SAT Solvers

libipasir_dummy.so: $(QUANTOR)/libipasir_dummy.so
	ln -sf $< .

# Always relink as the selected SATSOLVER might have changed
.PHONY: libipasir.so
libipasir.so: $(SATSOLVERLIBS)
	for i in $^; do ln -sf $$i .; done; rm -f $@; ln -s $< $@

# PicoSat
%/libipasir_picosat.so: %/makefile
	$(MAKE) -C $* $(notdir $@)

%/makefile: %.tar.gz
	rm -rf $*/ && tar xzf $< && cd $* && \
	patch -p1 < ../picosat-965_ipasir.patch && \
	./configure.sh -shared

picosat-%.tar.gz:
	wget http://fmv.jku.at/picosat/$@

# Riss
$(RISS)/lib%: $(RISS)/$(RISS);

%/$(RISS): %.tar.gz
	tar xzf $< && cd $* && make shared && touch $*
