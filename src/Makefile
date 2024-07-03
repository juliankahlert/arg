# Compiler and flags
CC       = $(shell which gcc || { echo cc; })
CFLAGS   = -Werror -Wall -Wextra -fPIC -flto -O3 -std=c11
AR       = ar
ARFLAGS  = rcs
SPARSE   = sparse
SPFLAGS  = -Wsparse-error -Wsparse-all -Wno-unknown-attribute \
           -fmax-errors=unlimited --fmax-warnings=unlimited
CKFMT    = clang-format --dry-run --Werror
LIB_NAME = arg
VERSION  = 1.0.0

# Directories
PREFIX = /usr/local
EXEC_PREFIX = $(PREFIX)
LIBDIR = $(EXEC_PREFIX)/lib
INCLUDEDIR = $(PREFIX)/include
PKGCONFIGDIR = $(LIBDIR)/pkgconfig

# Build directory
BUILD_DIR ?= .

# Source and object files
SRC = arg.c
OBJ = $(BUILD_DIR)/arg.o

# Targets
all: $(BUILD_DIR)/lib$(LIB_NAME).a $(BUILD_DIR)/lib$(LIB_NAME).so $(BUILD_DIR)/$(LIB_NAME).pc

# Static library
$(BUILD_DIR)/lib$(LIB_NAME).a: $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

# Shared library
$(BUILD_DIR)/lib$(LIB_NAME).so: $(OBJ)
	$(CC) -shared -o $@ $^

# Object file
$(OBJ): $(SRC) arg.h | $(BUILD_DIR)
	$(CKFMT) $<
	$(SPARSE) $(SPFLAGS) $(CFLAGS) -c $< -o $@
	$(CC) $(CFLAGS) -c $< -o $@

# Generate arg.pc from arg.pc.in
$(BUILD_DIR)/$(LIB_NAME).pc: $(LIB_NAME).pc.in | $(BUILD_DIR)
	sed \
		-e 's|@prefix@|$(PREFIX)|g' \
		-e 's|@exec_prefix@|$(EXEC_PREFIX)|g' \
		-e 's|@libdir@|$(LIBDIR)|g' \
		-e 's|@includedir@|$(INCLUDEDIR)|g' \
		-e 's|@VERSION@|$(VERSION)|g' \
		$< > $@

# Install
install: all
	install -d $(LIBDIR)
	install -d $(INCLUDEDIR)
	install -d $(PKGCONFIGDIR)
	install -m 644 $(BUILD_DIR)/lib$(LIB_NAME).a $(LIBDIR)
	install -m 755 $(BUILD_DIR)/lib$(LIB_NAME).so $(LIBDIR)
	install -m 644 arg.h $(INCLUDEDIR)
	install -m 644 $(BUILD_DIR)/$(LIB_NAME).pc $(PKGCONFIGDIR)

# Ensure build directory exists
$(BUILD_DIR):
	mkdir --parents $(BUILD_DIR)

# Clean up
clean:
	rm --force $(OBJ)
	rm --force $(BUILD_DIR)/lib$(LIB_NAME).a
	rm --force $(BUILD_DIR)/lib$(LIB_NAME).so
	rm --force $(BUILD_DIR)/$(LIB_NAME).pc

.PHONY: all clean install
