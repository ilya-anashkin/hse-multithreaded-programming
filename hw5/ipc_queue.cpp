#include "ipc_queue.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace hw5 {
namespace {

constexpr std::uint32_t kMagic = 128128128;
constexpr std::uint32_t kProtocolVersion = 1;
constexpr std::size_t kAlignment = 8;
constexpr std::uint16_t kPaddingFlag = 1;

std::size_t align_up(std::size_t value, std::size_t alignment) {
  return (value + alignment - 1) & ~(alignment - 1);
}

struct alignas(64) QueueHeader {
  std::uint64_t magic;
  std::uint32_t version;
  std::uint32_t capacity;
  std::atomic<std::uint64_t> head;
  std::atomic<std::uint64_t> tail;
};

struct alignas(8) RecordHeader {
  std::atomic<std::uint64_t> start;
  std::atomic<std::uint32_t> ready_size;
  std::uint16_t type;
  std::uint16_t flags;
  std::uint32_t payload_size;
};

constexpr std::size_t kRecordHeaderSize = sizeof(RecordHeader);

struct SharedMemoryRegion {
  int fd = -1;
  void* mapping = MAP_FAILED;
  std::size_t mapping_size = 0;
  std::string path;
  bool is_owner = false;

  SharedMemoryRegion() = default;
  SharedMemoryRegion(const SharedMemoryRegion&) = delete;
  SharedMemoryRegion& operator=(const SharedMemoryRegion&) = delete;

  SharedMemoryRegion(SharedMemoryRegion&& other) noexcept {
    *this = std::move(other);
  }

  SharedMemoryRegion& operator=(SharedMemoryRegion&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    close_region();
    fd = std::exchange(other.fd, -1);
    mapping = std::exchange(other.mapping, MAP_FAILED);
    mapping_size = std::exchange(other.mapping_size, 0);
    path = std::move(other.path);
    is_owner = std::exchange(other.is_owner, false);
    return *this;
  }

  ~SharedMemoryRegion() { close_region(); }

  void close_region() {
    if (mapping != MAP_FAILED) {
      munmap(mapping, mapping_size);
      mapping = MAP_FAILED;
    }
    if (fd != -1) {
      close(fd);
      fd = -1;
    }
    mapping_size = 0;
  }

  void unlink_if_owner() {
    if (is_owner && !path.empty()) {
      shm_unlink(path.c_str());
      is_owner = false;
    }
  }
};

std::runtime_error make_error(const std::string& what) {
  return std::runtime_error(what + ": " + std::strerror(errno));
}

SharedMemoryRegion open_region(const std::string& path);

SharedMemoryRegion create_region(const std::string& path,
                                 std::size_t queue_size) {
  const std::size_t aligned_queue_size = align_up(queue_size, kAlignment);
  if (aligned_queue_size < kRecordHeaderSize * 2) {
    throw std::invalid_argument("queue_size is too small");
  }

  SharedMemoryRegion region;
  if (path.empty() || path.front() != '/') {
    throw std::invalid_argument("path must be non-empty and start with '/'");
  }
  region.path = path;
  region.mapping_size = sizeof(QueueHeader) + aligned_queue_size;

  region.fd = shm_open(region.path.c_str(), O_CREAT | O_EXCL | O_RDWR, 0666);
  if (region.fd != -1) {
    region.is_owner = true;
    if (ftruncate(region.fd, static_cast<off_t>(region.mapping_size)) == -1) {
      throw make_error("ftruncate");
    }
    region.mapping = mmap(nullptr, region.mapping_size, PROT_READ | PROT_WRITE,
                          MAP_SHARED, region.fd, 0);
    if (region.mapping == MAP_FAILED) {
      throw make_error("mmap(create)");
    }

    std::memset(region.mapping, 0, region.mapping_size);
    auto* header = static_cast<QueueHeader*>(region.mapping);
    header->magic = kMagic;
    header->version = kProtocolVersion;
    header->capacity = static_cast<std::uint32_t>(aligned_queue_size);
    header->head.store(0, std::memory_order_relaxed);
    header->tail.store(0, std::memory_order_relaxed);
    return region;
  }

  if (errno != EEXIST) {
    throw make_error("shm_open(create)");
  }

  region = open_region(region.path);
  if (region.mapping_size < sizeof(QueueHeader) + aligned_queue_size) {
    throw std::runtime_error("existing queue has different size");
  }
  return region;
}

SharedMemoryRegion open_region(const std::string& path) {
  SharedMemoryRegion region;
  if (path.empty() || path.front() != '/') {
    throw std::invalid_argument("path must be non-empty and start with '/'");
  }
  region.path = path;

  region.fd = shm_open(region.path.c_str(), O_RDWR, 0666);
  if (region.fd == -1) {
    throw make_error("shm_open(open)");
  }

  struct stat st{};
  if (fstat(region.fd, &st) == -1) {
    throw make_error("fstat");
  }
  region.mapping_size = static_cast<std::size_t>(st.st_size);
  region.mapping = mmap(nullptr, region.mapping_size, PROT_READ | PROT_WRITE,
                        MAP_SHARED, region.fd, 0);
  if (region.mapping == MAP_FAILED) {
    throw make_error("mmap(open)");
  }

  auto* header = static_cast<QueueHeader*>(region.mapping);
  if (header->magic != kMagic) {
    throw std::runtime_error("shared memory contains unknown queue format");
  }
  if (header->version != kProtocolVersion) {
    throw std::runtime_error("shared memory protocol version mismatch");
  }
  if (region.mapping_size < sizeof(QueueHeader) + header->capacity) {
    throw std::runtime_error("shared memory size mismatch");
  }

  return region;
}

RecordHeader* record_at(QueueHeader* header, std::uint64_t position) {
  auto* buffer = reinterpret_cast<std::byte*>(header + 1);
  const auto offset = static_cast<std::size_t>(position % header->capacity);
  return reinterpret_cast<RecordHeader*>(buffer + offset);
}

const RecordHeader* record_at(const QueueHeader* header,
                              std::uint64_t position) {
  auto* buffer = reinterpret_cast<const std::byte*>(header + 1);
  const auto offset = static_cast<std::size_t>(position % header->capacity);
  return reinterpret_cast<const RecordHeader*>(buffer + offset);
}

std::byte* payload_at(RecordHeader* header) {
  return reinterpret_cast<std::byte*>(header) + sizeof(RecordHeader);
}

const std::byte* payload_at(const RecordHeader* header) {
  return reinterpret_cast<const std::byte*>(header) + sizeof(RecordHeader);
}

}  // namespace

ProducerNode::ProducerNode(std::string path, std::size_t queue_size)
    : path_(std::move(path)) {
  auto region = create_region(path_, queue_size);
  fd_ = region.fd;
  mapping_ = region.mapping;
  header_ = region.mapping;
  mapping_size_ = region.mapping_size;
  is_owner_ = region.is_owner;

  region.fd = -1;
  region.mapping = MAP_FAILED;
  region.mapping_size = 0;
  region.is_owner = false;
}

ProducerNode::~ProducerNode() {
  if (is_owner_ && !path_.empty()) {
    shm_unlink(path_.c_str());
  }
  if (mapping_ != nullptr && mapping_ != MAP_FAILED) {
    munmap(mapping_, mapping_size_);
  }
  if (fd_ != -1) {
    close(fd_);
  }
}

bool ProducerNode::push(std::uint16_t type, const void* data,
                        std::size_t size) {
  if (data == nullptr && size != 0) {
    throw std::invalid_argument("data is null while size is non-zero");
  }

  auto* header = static_cast<QueueHeader*>(header_);
  const auto needed = align_up(kRecordHeaderSize + size, kAlignment);
  if (needed >= header->capacity) {
    return false;
  }

  while (true) {
    const auto head = header->head.load(std::memory_order_acquire);
    const auto tail = header->tail.load(std::memory_order_relaxed);
    const auto offset = static_cast<std::size_t>(tail % header->capacity);
    const auto remaining = header->capacity - offset;

    std::size_t reserve = needed;
    bool has_padding_record = false;

    if (remaining < kRecordHeaderSize) {
      reserve += remaining;
    } else if (remaining < needed) {
      reserve += remaining;
      has_padding_record = true;
    }

    if (tail + reserve - head > header->capacity) {
      return false;
    }

    auto expected_tail = tail;
    if (!header->tail.compare_exchange_weak(expected_tail, tail + reserve,
                                            std::memory_order_acq_rel,
                                            std::memory_order_relaxed)) {
      continue;
    }

    if (has_padding_record) {
      auto* padding = record_at(header, tail);
      padding->ready_size.store(0, std::memory_order_relaxed);
      padding->start.store(tail, std::memory_order_relaxed);
      padding->type = 0;
      padding->flags = kPaddingFlag;
      padding->payload_size = 0;
      padding->ready_size.store(static_cast<std::uint32_t>(remaining),
                                std::memory_order_release);
    }

    const auto message_pos = tail + (reserve - needed);
    auto* record = record_at(header, message_pos);
    record->ready_size.store(0, std::memory_order_relaxed);
    record->start.store(message_pos, std::memory_order_relaxed);
    record->type = type;
    record->flags = 0;
    record->payload_size = static_cast<std::uint32_t>(size);
    if (size != 0) {
      std::memcpy(payload_at(record), data, size);
    }
    record->ready_size.store(static_cast<std::uint32_t>(needed),
                             std::memory_order_release);
    return true;
  }
}

bool ProducerNode::push(std::uint16_t type, std::string_view text) {
  return push(type, text.data(), text.size());
}

ConsumerNode::ConsumerNode(std::string path) : path_(std::move(path)) {
  auto region = open_region(path_);
  fd_ = region.fd;
  mapping_ = region.mapping;
  header_ = region.mapping;
  mapping_size_ = region.mapping_size;

  region.fd = -1;
  region.mapping = MAP_FAILED;
  region.mapping_size = 0;
}

ConsumerNode::~ConsumerNode() {
  if (mapping_ != nullptr && mapping_ != MAP_FAILED) {
    munmap(mapping_, mapping_size_);
  }
  if (fd_ != -1) {
    close(fd_);
  }
}

bool ConsumerNode::pop(std::uint16_t expected_type, MessageView& out) {
  auto* header = static_cast<QueueHeader*>(header_);
  while (true) {
    auto head = header->head.load(std::memory_order_relaxed);
    const auto tail = header->tail.load(std::memory_order_acquire);
    if (head == tail) {
      return false;
    }

    const auto offset = static_cast<std::size_t>(head % header->capacity);
    const auto remaining = header->capacity - offset;

    if (remaining < kRecordHeaderSize) {
      header->head.compare_exchange_strong(head, head + remaining,
                                           std::memory_order_release,
                                           std::memory_order_relaxed);
      continue;
    }

    const auto* record = record_at(header, head);
    if (record->start.load(std::memory_order_acquire) != head) {
      return false;
    }

    const auto ready_size = record->ready_size.load(std::memory_order_acquire);
    if (ready_size == 0) {
      return false;
    }

    if (record->flags & kPaddingFlag) {
      header->head.compare_exchange_strong(head, head + ready_size,
                                           std::memory_order_release,
                                           std::memory_order_relaxed);
      continue;
    }

    const auto payload_size = static_cast<std::size_t>(record->payload_size);
    if (ready_size < kRecordHeaderSize ||
        ready_size < kRecordHeaderSize + payload_size) {
      throw std::runtime_error("corrupted record in shared memory queue");
    }

    if (!header->head.compare_exchange_strong(head, head + ready_size,
                                              std::memory_order_release,
                                              std::memory_order_relaxed)) {
      continue;
    }

    if (record->type != expected_type) {
      continue;
    }

    out.type = record->type;
    out.payload.assign(payload_at(record), payload_at(record) + payload_size);
    return true;
  }
}

}  // namespace hw5
