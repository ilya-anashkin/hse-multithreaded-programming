#include <iostream>
#include <thread>

#include "buffered_channel.h"

int main() {
  BufferedChannel<int> ch(2);

  std::thread producer([&ch]() {
    for (int i = 1; i <= 5; ++i) {
      ch.Send(i);
    }
    ch.Close();
  });

  int sum = 0;
  while (auto value = ch.Recv()) {
    sum += *value;
  }

  producer.join();
  std::cout << "sum=" << sum << '\n';
  return 0;
}
