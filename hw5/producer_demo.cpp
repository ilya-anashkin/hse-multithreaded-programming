#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#include "ipc_queue.h"

namespace {

volatile std::sig_atomic_t g_stop = 0;

void handle_signal(int) { g_stop = 1; }

}  // namespace

int main(int argc, char** argv) {
  if (argc < 4) {
    std::cerr << "Usage: " << argv[0] << " <shm_path> <queue_size> <type>\n";
    return 1;
  }

  const std::string path = argv[1];
  const std::size_t queue_size = std::stoull(argv[2]);
  const std::uint16_t type = static_cast<std::uint16_t>(std::stoul(argv[3]));

  try {
    std::signal(SIGINT, handle_signal);
    hw5::ProducerNode producer(path, queue_size);

    while (!g_stop) {
      std::string payload;
      std::cout << "enter message: " << std::flush;

      if (!std::getline(std::cin, payload)) {
        if (g_stop) {
          break;
        }
        if (std::cin.eof()) {
          std::cin.clear();
          std::this_thread::sleep_for(std::chrono::milliseconds(50));
          continue;
        }
        std::cerr << "failed to read from stdin\n";
        return 1;
      }

      while (!g_stop && !producer.push(type, payload)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }

      if (!g_stop) {
        std::cout << "sent: " << payload << '\n';
      }
    }
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }

  return 0;
}
