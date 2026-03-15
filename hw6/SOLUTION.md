# SOLUTION

## Команды

```bash
make build
make test
make clean
```

## test

```bash
make test
```

Полученный мною результат:
```bash
    Start 1: ThreadPool.ComputesResults
1/7 Test #1: ThreadPool.ComputesResults .................   Passed    0.02 sec
    Start 2: ThreadPool.SupportsVoidTasks
2/7 Test #2: ThreadPool.SupportsVoidTasks ...............   Passed    0.00 sec
    Start 3: ThreadPool.WaitAndReadinessWork
3/7 Test #3: ThreadPool.WaitAndReadinessWork ............   Passed    0.04 sec
    Start 4: ThreadPool.PropagatesExceptions
4/7 Test #4: ThreadPool.PropagatesExceptions ............   Passed    0.00 sec
    Start 5: ThreadPool.NestedSubmitWorks
5/7 Test #5: ThreadPool.NestedSubmitWorks ...............   Passed    0.00 sec
    Start 6: ThreadPool.DestructorWaitsForQueuedTasks
6/7 Test #6: ThreadPool.DestructorWaitsForQueuedTasks ...   Passed    0.03 sec
    Start 7: Future.GetInvalidatesState
7/7 Test #7: Future.GetInvalidatesState .................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 7
```
