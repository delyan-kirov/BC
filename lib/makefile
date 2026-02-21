PROJECT_DIR = /home/dk/Projects/BC/
PREFIX ?= $(PROJECT_DIR)bin/

FFI_HEADER = $(PROJECT_DIR)inc/ffi.h
FFI_TARGET_HEADER = $(PROJECT_DIR)inc/ffitarget.h

LIBFFI_OBJ = $(PREFIX)lib64/libffi.so
LIBFFI_DIR = /home/dk/Projects/BC/lib/libffi/

RAYLIB_BLD = $(PROJECT_DIR)lib/raylib/build/
RAYLIB_HEADER = $(PROJECT_DIR)inc/raylib.h
RAYLIB_OBJ = $(PROJECT_DIR)bin/raylib.so

.PHONY: all raylib_examples configure_libffi build_libffi configure_raylib

all: $(RAYLIB_BLD) \
	$(RAYLIB_HEADER) \
	$(RAYLIB_OBJ) \
	raylib_examples \
	$(LIBFFI_OBJ) \
	$(FFI_HEADER) \
	$(FFI_TARGET_HEADER)

configure_libffi:
	cd $(LIBFFI_DIR) && ./configure --prefix=$(PREFIX)

build_libffi: configure_libffi
	$(MAKE) -C $(LIBFFI_DIR)

configure_raylib:
	cd $(PROJECT_DIR)lib/raylib/ \
	&& rm -rf build \
	&& mkdir build

$(RAYLIB_BLD): configure_raylib
	cd $(PROJECT_DIR)lib/raylib/build \
	&& cmake -DPLATFORM=Desktop -DBUILD_SHARED_LIBS=ON .. \
	&& $(MAKE)

$(RAYLIB_HEADER): $(RAYLIB_BLD)
	cp $(RAYLIB_BLD)raylib/include/raylib.h $(RAYLIB_HEADER)

$(RAYLIB_OBJ): $(RAYLIB_BLD)
	cp $(RAYLIB_BLD)raylib/libraylib.so $(RAYLIB_OBJ)

raylib_examples: $(RAYLIB_BLD)
	cd $(PROJECT_DIR)lib/raylib/build \
	&& cmake -DCUSTOMIZE_BUILD=ON -DBUILD_EXAMPLES=ON .. \
	&& $(MAKE)

$(LIBFFI_OBJ): build_libffi
	$(MAKE) -C $(LIBFFI_DIR) install

$(FFI_HEADER):
	cd $(PROJECT_DIR) && cp $(PREFIX)include/ffi.h $(FFI_HEADER)

$(FFI_TARGET_HEADER):
	cd $(PROJECT_DIR) && cp $(PREFIX)include/ffitarget.h $(FFI_TARGET_HEADER)
