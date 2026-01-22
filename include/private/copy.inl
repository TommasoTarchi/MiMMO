/**
 * @file copy.inl
 *
 * @brief Definition of methods for copying dual array data from host
 * to device and vice versa.
 */

#pragma once

namespace MiMMO {

/**
 * @brief Copies data from host to device.
 *
 * @details
 * This function copies data from host to device for a given dual array.
 *
 * The array must have been previously allocated on both host and device.
 *
 * @param dual_array   Dual array to synchronize.
 * @param offset       Index of first element to be copied.
 * @param num_elements Number of elements to be copied.
 */
template <typename T>
void DualMemoryManager::copy_host_to_device(DualArray<T> dual_array,
                                            const size_t offset,
                                            const size_t num_elements) {
  /* check that host pointer is initialized */
  if (dual_array.host_ptr == nullptr)
    abort_manager(dual_array.label + "'s host pointer is a null pointer.");

#ifdef _OPENACC
  /* check that device pointer is initialized */
  if (dual_array.dev_ptr == nullptr)
    abort_manager(dual_array.label + "'s device pointer is a null pointer.");

  /* copy data from host to device */
  acc_memcpy_to_device(dual_array.dev_ptr + offset,
                       dual_array.host_ptr + offset,
                       num_elements * sizeof(*(dual_array.host_ptr)));
#endif

  return;
}

/**
 * @brief Copies data from device to host.
 *
 * @details
 * This function copies data from device to host for a given dual array.
 *
 * The array must have been previously allocated on both host and device.
 *
 * @param dual_array   Dual array to synchronize.
 * @param offset       Index of first element to be copied.
 * @param num_elements Number of elements to be copied.
 */
template <typename T>
void DualMemoryManager::copy_device_to_host(DualArray<T> dual_array,
                                            const size_t offset,
                                            const size_t num_elements) {
  /* check that host pointer is initialized */
  if (dual_array.host_ptr == nullptr)
    abort_manager(dual_array.label + "'s host pointer is a null pointer.");

#ifdef _OPENACC
  /* check that device pointer is initialized */
  if (dual_array.dev_ptr == nullptr)
    abort_manager(dual_array.label + "'s device pointer is a null pointer.");

  /* copy data from device to host */
  acc_memcpy_from_device(dual_array.host_ptr + offset,
                         dual_array.dev_ptr + offset,
                         num_elements * sizeof(*(dual_array.host_ptr)));
#endif

  return;
}

} // namespace MiMMO
