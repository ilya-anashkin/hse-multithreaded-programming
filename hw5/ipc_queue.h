#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace hw5 {

struct MessageView {
  std::uint16_t type = 0;
  std::vector<std::byte> payload;
};

class ProducerNode {
 public:
  ProducerNode(std::string path, std::size_t queue_size);
  ~ProducerNode();

  bool can_fit(std::size_t size) const;
  bool push(std::uint16_t type, const void* data, std::size_t size);
  bool push(std::uint16_t type, std::string_view text);

 private:
  int fd_ = -1;
  void* mapping_ = nullptr;
  void* header_ = nullptr;
  std::size_t mapping_size_ = 0;
  std::string path_;
  bool is_owner_ = false;
};

class ConsumerNode {
 public:
  explicit ConsumerNode(std::string path);
  ~ConsumerNode();

  bool pop(std::uint16_t expected_type, MessageView& out);

 private:
  int fd_ = -1;
  void* mapping_ = nullptr;
  void* header_ = nullptr;
  std::size_t mapping_size_ = 0;
  std::string path_;
};

}  // namespace hw5
