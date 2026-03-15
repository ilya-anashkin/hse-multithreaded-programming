#include <chrono>
#include <csignal>
#include <cstddef>
#include <iostream>
#include <string>
#include <thread>

#include "ipc_queue.h"

namespace {

volatile std::sig_atomic_t g_stop = 0;

void handle_signal(int) { g_stop = 1; }

}  // namespace

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <shm_path> <expected_type>\n";
    return 1;
  }

  const std::string path = argv[1];
  const std::uint16_t expected_type =
      static_cast<std::uint16_t>(std::stoul(argv[2]));

  try {
    std::signal(SIGINT, handle_signal);
    hw5::ConsumerNode consumer(path);

    while (!g_stop) {
      hw5::MessageView message;
      if (!consumer.pop(expected_type, message)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        continue;
      }

      const std::string text(
          reinterpret_cast<const char*>(message.payload.data()),
          message.payload.size());
      std::cout << "received: " << text << '\n';
    }
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }

  return 0;
}
