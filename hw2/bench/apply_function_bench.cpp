#include <benchmark/benchmark.h>

#include <apply_function.hpp>
#include <cmath>
#include <vector>

static void BM_SingleThreadUsuallyFaster(benchmark::State& state) {
  const int dataSize = static_cast<int>(state.range(0));
  const int threadCount = static_cast<int>(state.range(1));

  for (auto _ : state) {
    std::vector<int> data(dataSize, 1);
    ApplyFunction<int>(data, [](int& value) { value += 1; }, threadCount);
    benchmark::DoNotOptimize(data.data());
  }

  state.SetItemsProcessed(state.iterations() * dataSize);
}

static void BM_MultiThreadUsuallyFaster(benchmark::State& state) {
  const int dataSize = static_cast<int>(state.range(0));
  const int threadCount = static_cast<int>(state.range(1));

  for (auto _ : state) {
    std::vector<double> data(dataSize, 1.001);
    ApplyFunction<double>(
        data,
        [](double& value) {
          double acc = value;
          for (int i = 0; i < 1500; ++i) {
            acc =
                std::sin(acc) + std::cos(acc) + std::sqrt(std::fabs(acc) + 1.0);
          }
          value = acc;
        },
        threadCount);
    benchmark::DoNotOptimize(data.data());
  }

  state.SetItemsProcessed(state.iterations() * dataSize);
}

BENCHMARK(BM_SingleThreadUsuallyFaster)->Args({128, 1})->Args({128, 8});
BENCHMARK(BM_MultiThreadUsuallyFaster)->Args({20000, 1})->Args({20000, 8});

BENCHMARK_MAIN();
