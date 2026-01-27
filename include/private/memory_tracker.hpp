/**
 * @file memory_tracker.hpp
 *
 * @brief Declaration of functions for updating the memory tracker.
 */

#pragma once

#include <map>
#include <string>

namespace MiMMO {

/**
 * @brief Adds an entry to the given memory tracker.
 *
 * @details
 * This function adds an array to the given memory tracker in the form
 * (label, (size, on_device)).
 *
 * If the label (key) is already present, the input is ignored and error
 * is returned.
 *
 * @param memory_tracker   Memory tracker to update.
 * @param tot_memory_usage Pair containing total host and device memory
 *                         usage.
 * @param label            Label of the array to be added.
 * @param size             Size in bytes of the array to be added.
 * @param on_device        Whether the array is allocated on device.
 *
 * @return                 'true' if the array was already tracked, 'false'
 *                         otherwise.
 */
bool add_to_memory_tracker(
    std::map<std::string, std::pair<size_t, bool>> &memory_tracker,
    std::pair<size_t, size_t> &tot_memory_usage, const std::string label,
    const size_t size, const bool on_device = false);

/**
 * @brief Removes an entry from the given memory tracker.
 *
 * @details
 * This function removes an array with a given label from the given memory
 * tracker.
 *
 * If the label (key) is not present, the input is ignored and error is
 * returned.
 *
 * @param memory_tracker   Memory tracker to update.
 * @param tot_memory_usage Pair containing total host and device memory
 *                         usage.
 * @param label            Label of the array to be added.
 *
 * @return                 'true' if the array was not tracked, 'false'
 *                         otherwise.
 */
bool remove_from_memory_tracker(
    std::map<std::string, std::pair<size_t, bool>> &memory_tracker,
    std::pair<size_t, size_t> &tot_memory_usage, const std::string label);

} // namespace MiMMO
