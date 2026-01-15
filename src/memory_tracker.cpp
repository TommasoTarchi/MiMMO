/**
 * @file memory_tracker.cpp
 *
 * @brief Implementation of functions for updating the memory tracker.
 */

#include "../include/private/memory_tracker.hpp"

namespace DualMemoryManager {

// TODO: add description
bool add_to_memory_tracker(
    std::map<std::string, std::pair<size_t, bool>> &memory_tracker,
    const std::string label, const size_t size, const bool on_device) {

  /* attempt to add new element */
  const auto ret = memory_tracker.insert({label, {size, on_device}});

  /* compute outcome */
  const bool out = !(ret.second);

  return out;
}

// TODO: add description
bool remove_from_memory_tracker(
    std::map<std::string, std::pair<size_t, bool>> &memory_tracker,
    const std::string label) {

  /* try to remove element */
  const int ret = memory_tracker.erase(label);

  /* compute outcome */
  const bool out = (ret == 0);

  return out;
}

} // namespace DualMemoryManager
