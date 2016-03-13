PICOSAT := picosat-965
QUANTOR := quantor-3.2

.PHONY: all clean

all: libpicosat.a libquantor.a

# PicoSat
libpicosat.a: $(PICOSAT)/libpicosat.a
	ln -s $< .

%/libpicosat.a: %.tar.gz
	tar xzf $< && cd $* && ./configure.sh && make

picosat-%.tar.gz:
	wget http://fmv.jku.at/picosat/$@

# Quantor
libquantor.a: $(QUANTOR)/libquantor.a
	ln -s $< .

%/libquantor.a: %.tar.gz $(PICOSAT)/libpicosat.a
	tar xzf $< && cd $* && ./configure && make

quantor-%.tar.gz:
	wget http://fmv.jku.at/quantor/$@

clean:
	rm -rf lib*.a
	$(MAKE) -C $(QUANTOR) clean
	$(MAKE) -C $(PICOSAT) clean
