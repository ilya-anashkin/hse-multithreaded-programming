# SOLUTION

## Команды

```bash
make run
make test
make bench
make clean
```

## test

```bash
make test
```

Полученный мною результат:
```bash
    Start 1: buffered_channel_test
1/1 Test #1: buffered_channel_test ............   Passed    2.15 sec

100% tests passed, 0 tests failed out of 1
```

## benchmark

```bash
make bench
```

Полученный мною результат:
```bash
Run on (12 X 24 MHz CPU s)
CPU Caches:
  L1 Data 64 KiB
  L1 Instruction 128 KiB
  L2 Unified 4096 KiB (x12)
Load Average: 2.86, 2.22, 2.14
-----------------------------------------------------------------------------------------------
Benchmark                                                     Time             CPU   Iterations
-----------------------------------------------------------------------------------------------
Run/2/2/10/min_time:0.100/process_time/real_time           2947 ms        15842 ms            1
Run/10/6/6/min_time:0.100/process_time/real_time           1339 ms        10755 ms            1
Run/100000/6/6/min_time:0.100/process_time/real_time       24.2 ms          229 ms            5
```