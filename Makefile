BUILD_DIR ?= $(shell realpath ./build)
TARGETS   := all clean install

export BUILD_DIR

.PHONY: $(TARGETS)

# Targets
$(TARGETS):
	$(MAKE) -C src $@
