BUILD_DIR := build

JOBS := $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
CMAKE := cmake

.PHONY: all build clean

all: build

$(BUILD_DIR)/Makefile:
	$(CMAKE) -S . -B $(BUILD_DIR)

build: $(BUILD_DIR)/Makefile
	$(CMAKE) --build $(BUILD_DIR) -j $(JOBS)

clean:
	rm -rf $(BUILD_DIR)
