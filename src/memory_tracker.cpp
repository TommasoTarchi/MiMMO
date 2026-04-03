/**
 * @file memory_tracker.cpp
 *
 * @brief Implementation of memory tracker functions.
 *
 * @see memory_tracker.hpp
 */

#include "../include/private/memory_tracker.hpp"

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
    const std::string label, const size_t size, const bool on_device) {

  /* attempt to add new element */
  const auto ret = memory_tracker.insert({object, {label, size, on_device}});

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
    std::pair<size_t, size_t> &tot_memory_usage, void *const object) {

  /* try to remove element */
  const auto ret = memory_tracker.extract(object);

  /* if element was not found, return error */
  if (ret.empty())
    return true;

  /* update total memory usage */
  const size_t size = std::get<1>(ret.mapped());
  const bool on_device = std::get<2>(ret.mapped());
  tot_memory_usage.first -= size;
  if (on_device)
    tot_memory_usage.second -= size;

  return false;
}

} // namespace MiMMO
