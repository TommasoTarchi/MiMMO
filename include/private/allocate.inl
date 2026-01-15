/**
 * @file allocate.inl
 *
 * @brief Definition of method for allocating dual arrays.
 */

#pragma once

namespace DualMemoryManager {

/**
 * @brief Allocates dual array memory.
 *
 * @details
 * This function allocates memory on host and (if requested) on device
 * for a certain array. It returns an object of type DualArray,
 * containing host and device pointers.
 *
 * @tparam T Type of elements in array to be allocated.
 *
 * @param label        Label that should be used to track the array in
 *                     memory.
 * @param num_elements Number of elements in the array.
 * @param on_device    Whether the array should be allocated on device
 *                     as well.
 *
 * @return Allocated array in the form of an object of type DualArray.
 */
template <typename T>
DualArray<T> DualMemoryManager::allocate(const std::string label,
                                         const size_t num_elements,
                                         const bool on_device) {
  DualArray<T> dual_array;

  /* check that device has not been required if openACC is not enabled */
#ifndef _OPENACC
  if (on_device)
    abort_manager(label + " was requested to be allocated on device as well, "
                          "but openACC is not enabled.");
#endif // _OPENACC

  /* allocate memory on host */
  dual_array.host_ptr = (T *)std::malloc(num_elements * sizeof(T));

  /* if required, allocate memory on device */
  if (on_device) {

#ifdef _OPENACC
    dual_array.dev_ptr = (T *)acc_malloc(num_elements * sizeof(T));
#endif // _OPENACC

    if (dual_array.dev_ptr == nullptr) {
      std::free(dual_array.host_ptr);
      dual_array.host_ptr = nullptr;
      abort_manager("Failed to allocate device memory.");
    }

  } else {
    dual_array.dev_ptr = nullptr;
  }

  /* update array label */
  dual_array.label = label;

  /* update number of elements and bytes */
  dual_array.num_elements = num_elements;
  dual_array.size = num_elements * sizeof(T);

  /* update used memory and memory tracker */
  total_host_memory += dual_array.size;
  if (on_device)
    total_device_memory += dual_array.size;

  const bool ret =
      add_to_memory_tracker(memory_tracker, label, dual_array.size, on_device);

  if (ret)
    abort_manager(label + " already exists. Please choose another label.");

  return dual_array;
}

} // namespace DualMemoryManager
