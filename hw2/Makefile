CMAKE := cmake
BUILD_DIR := build

.PHONY: build run test bench clean

build:
	$(CMAKE) -S . -B $(BUILD_DIR)
	$(CMAKE) --build $(BUILD_DIR)

run: build
	$(CMAKE) --build $(BUILD_DIR) --target run

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

bench: build
	$(CMAKE) --build $(BUILD_DIR) --target bench

clean:
	$(CMAKE) --build $(BUILD_DIR) --target clean || true
	rm -rf $(BUILD_DIR)
