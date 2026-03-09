#include <apply_function.hpp>
#include <iostream>
#include <vector>

int main() {
  std::vector<int> data{1, 2, 3, 4, 5};

  ApplyFunction<int>(data, [](int& value) { value *= 3; }, 3);

  for (const int value : data) {
    std::cout << value << ' ';
  }
  std::cout << '\n';

  return 0;
}
