# SOLUTION

## Команды

```bash
make build
make clean
```

## Demo

```bash
./build/producer_demo /hw5-queue 100 42

enter message: asdf
sent: asdf
enter message: test
sent: test
enter message: jkl
sent: jkl
```

```bash
./build/consumer_demo /hw5-queue 42

received: asdf
received: test
received: jkl
```
