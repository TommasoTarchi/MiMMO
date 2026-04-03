/**
 * @file memory_tracker.hpp
 *
 * @brief Declaration of functions for the memory tracker.
 *
 * Internal utilities for tracking allocated memory on host and device.
 * Used by DualMemoryManager to maintain memory usage reports.
 *
 * @see memory_tracker.cpp for implementations
 */

#pragma once

#include <map>
#include <string>

namespace MiMMO {

/**
 * @brief Adds an entry to the given memory tracker.
 *
 * @param memory_tracker   Memory tracker to update.
 * @param tot_memory_usage Pair containing total host and device memory
 *                         usage.
 * @param object           Pointer to dual object to be added.
 * @param label            Label of the array to be added.
 * @param size             Size in bytes of the array to be added.
 * @param on_device        Whether the array is allocated on device.
 *
 * @return                 'true' if the array was already tracked, 'false'
 *                         otherwise.
 *
 * @note If the label (key) is already present, the entry is ignored.
 */
bool add_to_memory_tracker(
    std::map<void *, std::tuple<std::string, size_t, bool>> &memory_tracker,
    std::pair<size_t, size_t> &tot_memory_usage, void *const object,
    const std::string label, const size_t size, const bool on_device);

/**
 * @brief Removes an entry from the given memory tracker.
 *
 * @param memory_tracker   Memory tracker to update.
 * @param tot_memory_usage Pair containing total host and device memory
 *                         usage.
 * @param object           Pointer to dual object to be removed.
 *
 * @return                 'true' if the array was not tracked, 'false'
 *                         otherwise.
 *
 * @note If the object is not tracked, the operation is ignored.
 */
bool remove_from_memory_tracker(
    std::map<void *, std::tuple<std::string, size_t, bool>> &memory_tracker,
    std::pair<size_t, size_t> &tot_memory_usage, void *const object);

} // namespace MiMMO
