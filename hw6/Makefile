BUILD_DIR := build

JOBS := $(shell nproc 2>/dev/null || sysctl -n hw.ncpu)
CMAKE := cmake
CTEST := ctest

.PHONY: all build test clean

all: build test

$(BUILD_DIR)/Makefile:
	$(CMAKE) -S . -B $(BUILD_DIR)

build: $(BUILD_DIR)/Makefile
	$(CMAKE) --build $(BUILD_DIR) -j $(JOBS)

test: build
	$(CTEST) --test-dir $(BUILD_DIR) --output-on-failure

clean:
	rm -rf $(BUILD_DIR)
