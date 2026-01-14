// TODO: add file description

#pragma once

#include <map>
#include <string>

namespace DualMemoryManager {

// TODO: add description
bool add_to_memory_tracker(std::map<std::string, size_t> &, const std::string,
                           const size_t);
// TODO: add description
bool remove_from_memory_tracker(std::map<std::string, size_t> &,
                                const std::string);

} // namespace DualMemoryManager
