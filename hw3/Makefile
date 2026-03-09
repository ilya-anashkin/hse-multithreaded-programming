APP_BUILD_DIR := build/app
TEST_BUILD_DIR := build/test
BENCH_BUILD_DIR := build/bench
CMAKE := cmake
CTEST := ctest

.PHONY: run test bench clean

run:
	$(CMAKE) -S . -B $(APP_BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=OFF -DENABLE_BENCHMARK=OFF
	$(CMAKE) --build $(APP_BUILD_DIR) -j
	./$(APP_BUILD_DIR)/buffered_channel_app

test:
	$(CMAKE) -S . -B $(TEST_BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=ON -DENABLE_BENCHMARK=OFF
	$(CMAKE) --build $(TEST_BUILD_DIR) -j
	$(CTEST) --test-dir $(TEST_BUILD_DIR) --output-on-failure

bench:
	$(CMAKE) -S . -B $(BENCH_BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=OFF -DENABLE_BENCHMARK=ON
	$(CMAKE) --build $(BENCH_BUILD_DIR) -j
	./$(BENCH_BUILD_DIR)/buffered_channel_benchmark --benchmark_min_time=0.05s

clean:
	$(CMAKE) -E rm -rf build
