# SOLUTION

## Команды

```bash
make build
make test
make bench
make run
make clean
```

## test

```bash
make test
```

Полученный мною результат:
```bash
    Start 1: ApplyFunctionTest.AppliesTransformSingleThread
1/6 Test #1: ApplyFunctionTest.AppliesTransformSingleThread ......................   Passed    0.00 sec
    Start 2: ApplyFunctionTest.AppliesTransformMultiThread
2/6 Test #2: ApplyFunctionTest.AppliesTransformMultiThread .......................   Passed    0.00 sec
    Start 3: ApplyFunctionTest.LimitsThreadCountByDataSize
3/6 Test #3: ApplyFunctionTest.LimitsThreadCountByDataSize .......................   Passed    0.00 sec
    Start 4: ApplyFunctionTest.HandlesZeroAndNegativeThreadCountAsSingleThread
4/6 Test #4: ApplyFunctionTest.HandlesZeroAndNegativeThreadCountAsSingleThread ...   Passed    0.00 sec
    Start 5: ApplyFunctionTest.HandlesEmptyVector
5/6 Test #5: ApplyFunctionTest.HandlesEmptyVector ................................   Passed    0.00 sec
    Start 6: ApplyFunctionTest.RethrowsTransformException
6/6 Test #6: ApplyFunctionTest.RethrowsTransformException ........................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 6
```

## benchmark

В benchmark есть два сценария:
- `BM_SingleThreadUsuallyFaster`: маленький вектор + лёгкий `transform` (выигрывает 1 поток из-за накладных расходов потоков)
- `BM_MultiThreadUsuallyFaster`: большой вектор + тяжёлый `transform` (выигрывают несколько потоков)

Полученный мною результат:
```bash
Run on (12 X 24 MHz CPU s)
CPU Caches:
  L1 Data 64 KiB
  L1 Instruction 128 KiB
  L2 Unified 4096 KiB (x12)
Load Average: 2.12, 2.13, 2.11
----------------------------------------------------------------------------------------------
Benchmark                                    Time             CPU   Iterations UserCounters...
----------------------------------------------------------------------------------------------
BM_SingleThreadUsuallyFaster/128/1        1751 ns         1750 ns       161246 items_per_second=73.1282M/s
BM_SingleThreadUsuallyFaster/128/8       62194 ns        60960 ns         4598 items_per_second=2.09972M/s
BM_MultiThreadUsuallyFaster/20000/1  499788875 ns    499130000 ns            1 items_per_second=40.0697k/s
BM_MultiThreadUsuallyFaster/20000/8   73003914 ns       197180 ns          100 items_per_second=101.43M/s
```
