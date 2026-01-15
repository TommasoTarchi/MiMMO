/**
 * @file memory_tracker.hpp
 *
 * @brief Declaration of functions for updating the memory tracker.
 */

#pragma once

#include <map>
#include <string>

namespace DualMemoryManager {

// TODO: add description
bool add_to_memory_tracker(std::map<std::string, std::pair<size_t, bool>> &,
                           const std::string, const size_t, const bool = false);
// TODO: add description
bool remove_from_memory_tracker(
    std::map<std::string, std::pair<size_t, bool>> &, const std::string);

} // namespace DualMemoryManager
