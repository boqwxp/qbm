BIN_TARGETS := qdlsolve

# Standard Targets
.PHONY: all libs clean $(BIN_TARGETS) FORCE
all: $(BIN_TARGETS)

clean:
	$(MAKE) -C lib/ clean
	$(MAKE) -C src/model clean
	$(MAKE) -C src/qdl clean
	rm -rf bin/

## Binary Targets ###########################################################
# Create bin/ Directory
$(BIN_TARGETS): bin
bin:
	mkdir -p $@

# Generic Target Rules
$(BIN_TARGETS): %: bin/%
bin/%:
	cp $< $@
src/%: FORCE
	$(MAKE) -C $(dir $@) $(notdir $@)

# Individual Dependencies
bin/qdlsolve: src/qdl/qdlsolve

## Libaries #################################################################
libs:
	$(MAKE) -C lib/
