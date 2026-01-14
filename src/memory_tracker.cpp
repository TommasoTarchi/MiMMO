// TODO: add file description

#include "../include/private/memory_tracker.hpp"

namespace DualMemoryManager {

// TODO: add description
bool add_to_memory_tracker(std::map<std::string, size_t> &memory_tracker,
                           const std::string label, const size_t size) {

  /* attempt to add new element */
  const auto ret =
      memory_tracker.insert(std::pair<std::string, size_t>(label, size));

  /* compute outcome */
  const bool out = !(ret.second);

  return out;
}

// TODO: add description
bool remove_from_memory_tracker(std::map<std::string, size_t> &memory_tracker,
                                const std::string label) {

  /* try to remove element */
  const int ret = memory_tracker.erase(label);

  /* compute outcome */
  const bool out = (ret == 0);

  return out;
}

} // namespace DualMemoryManager
