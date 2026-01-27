/**
 * @file memory_tracker.cpp
 *
 * @brief Implementation of functions for updating the memory tracker.
 */

#include "../include/private/memory_tracker.hpp"

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
    const size_t size, const bool on_device) {

  /* attempt to add new element */
  const auto ret = memory_tracker.insert({label, {size, on_device}});

  /* if element was already present, return error */
  if (!(ret.second))
    return true;

  /* update total memory usage */
  tot_memory_usage.first += size;
  if (on_device)
    tot_memory_usage.second += size;

  return false;
}

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
    std::pair<size_t, size_t> &tot_memory_usage, const std::string label) {

  /* try to remove element */
  const auto ret = memory_tracker.extract(label);

  /* if element was not found, return error */
  if (ret.empty())
    return true;

  /* update total memory usage */
  const size_t size = ret.mapped().first;
  const bool on_device = ret.mapped().second;
  tot_memory_usage.first -= size;
  if (on_device)
    tot_memory_usage.second -= size;

  return false;
}

} // namespace MiMMO
