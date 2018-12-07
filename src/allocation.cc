struct {
  std::size_t usage, peak_usage;
  std::size_t allocations, deallocations;
} allocation_info;

struct AllocationMetadata {
  std::size_t size;
#ifndef NDEBUG
  std::size_t checksum;
#endif  // NDEBUG
};

void* operator new(std::size_t size) {
  allocation_info.allocations++;
  allocation_info.usage += size;
  if (allocation_info.usage > allocation_info.peak_usage)
    allocation_info.peak_usage = allocation_info.usage;
  void* p = std::malloc(sizeof(AllocationMetadata) + size);
  auto* metadata = reinterpret_cast<AllocationMetadata*>(p);
  metadata->size = size;
#ifndef NDEBUG
  metadata->checksum = reinterpret_cast<std::uintptr_t>(p) ^ size;
#endif  // NDEBUG
  return reinterpret_cast<char*>(p) + sizeof(AllocationMetadata);
}

void operator delete(void* p) noexcept {
  p = reinterpret_cast<char*>(p) - sizeof(AllocationMetadata);
  auto* metadata = reinterpret_cast<AllocationMetadata*>(p);
#ifndef NDEBUG
  std::size_t checksum = reinterpret_cast<std::uintptr_t>(p) ^ metadata->size;
  assert(metadata->checksum == checksum);
#endif  // NDEBUG
  allocation_info.deallocations++;
  allocation_info.usage -= metadata->size;
  std::free(p);
}

void print_bytes(std::size_t count) {
  std::cout << count << "B";
  if (count > 5000000) {
    std::cout << " (~" << count / 1000000 << "MB)";
  } else if (count > 5000) {
    std::cout << " (~" << count / 1000 << "KB)";
  }
}

void dump_allocation_stats() {
  auto info = allocation_info;
  std::cout << info.allocations << " allocations, peak usage ";
  print_bytes(info.peak_usage);
  std::cout << ".\n";
  if (info.usage != 0) {
    std::cout << "\x1b[31m";
    print_bytes(info.usage);
    std::cout << " allocated at exit.\x1b[0m\n";
  }
  if (info.allocations != info.deallocations) {
    std::cout << "\x1b[31m";
    std::cout << info.deallocations << " deallocations vs. "
              << info.allocations << " allocations.\x1b[0m\n";
  }
}
