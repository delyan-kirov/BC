PROJECT_DIR = /home/dk/Projects/BC/
PREFIX ?= $(PROJECT_DIR)bin/
FFI_HEADER = $(PROJECT_DIR)inc/ffi.h
FFI_TARGET_HEADER = $(PROJECT_DIR)inc/ffitarget.h

LIBFFI_OBJ = $(PREFIX)lib64/libffi.so
LIBFFI_DIR = /home/dk/Projects/BC/lib/libffi/

.PHONY: all configure build

all: $(LIBFFI_OBJ) $(FFI_HEADER) $(FFI_TARGET_HEADER)

configure:
	cd $(LIBFFI_DIR) && ./configure --prefix=$(PREFIX)

build: configure
	$(MAKE) -C $(LIBFFI_DIR)


$(LIBFFI_OBJ): build
	$(MAKE) -C $(LIBFFI_DIR) install

$(FFI_HEADER):
	cd $(PROJECT_DIR) && cp $(PREFIX)include/ffi.h $(FFI_HEADER)

$(FFI_TARGET_HEADER):
	cd $(PROJECT_DIR) && cp $(PREFIX)include/ffitarget.h $(FFI_TARGET_HEADER)
