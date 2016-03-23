QUANTOR := quantor-3.2

# Available SAT Solvers
PICOSAT := picosat-965
IPASIR  := riss_505

# Choose one of them
SATSOLVER := picosat
SATSOLVER_DIR := $($(shell echo $(SATSOLVER) | tr '[a-z]' '[A-Z]'))

.PHONY: all clean clobber

all: libquantor.a libsatsolver.a

#############################################################################
# Quantor
libquantor.a: $(QUANTOR)/libquantor.a
	ln -sf $< .

%/libquantor.a: %.tar.gz $(SATSOLVER_DIR)/lib$(SATSOLVER).a
	tar xzf $< && cd $* && \
	patch -p1 < ../quantor-3.2_ipasir.patch && \
	./configure --$(SATSOLVER)=../$(SATSOLVER_DIR) && $(MAKE) $(notdir $@)

quantor-%.tar.gz:
	wget http://fmv.jku.at/quantor/$@

#############################################################################
# SAT Solvers

libsatsolver.a: $(SATSOLVER_DIR)/lib$(SATSOLVER).a
	ln -sf $< $@

# PicoSat
%/libpicosat.a: %.tar.gz
	tar xzf $< && cd $* && ./configure.sh && $(MAKE)

picosat-%.tar.gz:
	wget http://fmv.jku.at/picosat/$@

# IPASIR: Riss
%/libipasir.a: $(IPASIR).tar.gz
	tar xzf $< && cd $* && $(MAKE) && ln -sf libipasir$(IPASIR).a libipasir.a

#############################################################################
clean:
	$(MAKE) -C $(QUANTOR) clean; \
	$(MAKE) -C $(PICOSAT) clean; \
	rm -rf lib*.a

clobber: clean
	rm -rf $(PICOSAT) $(IPASIR) $(QUANTOR)
