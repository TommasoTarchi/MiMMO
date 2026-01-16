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
 * @param dual_array Dual array to synchronize.
 */
template <typename T>
void DualMemoryManager::copy_host_to_device(DualArray<T> dual_array) {
  /* check that host and device pointers are initialized */
  if (dual_array.host_ptr == nullptr)
    abort_manager(dual_array.label + "'s host pointer is a null pointer.");
  if (dual_array.host_ptr == nullptr)
    abort_manager(dual_array.label + "'s device pointer is a null pointer.");

  /* copy data from host to device */
#ifdef _OPENACC
  acc_memcpy_to_device(dual_array.dev_ptr, dual_array.host_ptr,
                       dual_array.size);
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
 * @param dual_array Dual array to synchronize.
 */
template <typename T>
void DualMemoryManager::copy_device_to_host(DualArray<T> dual_array) {
  /* check that host and device pointers are initialized */
  if (dual_array.host_ptr == nullptr)
    abort_manager(dual_array.label + "'s host pointer is a null pointer.");
  if (dual_array.host_ptr == nullptr)
    abort_manager(dual_array.label + "'s device pointer is a null pointer.");

  /* copy data from device to host */
#ifdef _OPENACC
  acc_memcpy_from_device(dual_array.host_ptr, dual_array.dev_ptr,
                         dual_array.size);
#endif

  return;
}

} // namespace MiMMO
