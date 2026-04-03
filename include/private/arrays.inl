/**
 * @file arrays.inl
 *
 * @brief Definition of template methods for managing dual arrays.
 *
 * Implements the following DualMemoryManager methods:
 * - alloc_array()
 * - update_array_host_to_device()
 * - update_array_device_to_host()
 * - free_array()
 *
 * @see api.hpp for the corresponding declarations
 */

#pragma once

namespace MiMMO {

/**
 * @brief Allocates dual array memory.
 *
 * @tparam T         Type of elements in the array.
 *
 * @param dual_array Dual array to be allocated.
 * @param label      Label that should be used to track the array in
 *                   memory.
 * @param size       Number of elements in the array.
 * @param on_device  Whether the array should be allocated on device as
 *                   well (ignored if main code compiled without OpenACC
 *                   support).
 */
template <typename T>
void DualMemoryManager::alloc_array(DualArray<T> &dual_array,
                                    const std::string label, const size_t size,
                                    const bool on_device) {

  /* allocate memory on host */
  dual_array.host_ptr = (T *)std::malloc(size * sizeof(T));

  /* if required, allocate memory on device */
#ifdef _OPENACC
  if (on_device) {
    dual_array.dev_ptr = (T *)acc_malloc(size * sizeof(T));

    if (!(dual_array.dev_ptr)) {
      std::free(dual_array.host_ptr);
      dual_array.host_ptr = nullptr;
      abort_mimmo("Failed to allocate device memory.");
    }

  } else {
    dual_array.dev_ptr = nullptr;
  }
#else
  dual_array.dev_ptr = nullptr;
#endif // _OPENACC

  /* update number of elements and bytes */
  dual_array.size = size;
  dual_array.size_bytes = size * sizeof(T);

  /* update memory tracker */
#ifdef _OPENACC
  const bool ret =
      add_to_memory_tracker(memory_tracker, total_memory, (void *)&dual_array,
                            label, dual_array.size_bytes, on_device);
#else
  const bool ret =
      add_to_memory_tracker(memory_tracker, total_memory, (void *)&dual_array,
                            label, dual_array.size_bytes, false);
#endif // _OPENACC

  if (ret)
    abort_mimmo("Failed to track memory for dual array '" + label + "'.");

  return;
}

/**
 * @brief Copies data from host to device.
 *
 * @tparam T           Type of elements in the array.
 *
 * @param dual_array   Dual array to synchronize.
 * @param offset       Index of first element to be copied.
 * @param num_elements Number of elements to be copied.
 *
 * @note If OpenACC is not enabled, this function does nothing.
 */
template <typename T>
void DualMemoryManager::update_array_host_to_device(DualArray<T> dual_array,
                                                    const size_t offset,
                                                    const size_t num_elements) {
  /* check that host pointer is initialized */
  if (dual_array.host_ptr == nullptr)
    abort_mimmo("Host pointer of dual array is a null pointer.");

#ifdef _OPENACC
  /* check that device pointer is initialized */
  if (dual_array.dev_ptr == nullptr)
    abort_mimmo("Device pointer of dual array is a null pointer.");

  /* copy data from host to device */
  acc_memcpy_to_device(dual_array.dev_ptr + offset,
                       dual_array.host_ptr + offset, num_elements * sizeof(T));
#endif // _OPENACC

  return;
}

/**
 * @brief Copies data from device to host.
 *
 * @tparam T           Type of elements in the array.
 *
 * @param dual_array   Dual array to synchronize.
 * @param offset       Index of first element to be copied.
 * @param num_elements Number of elements to be copied.
 *
 * @note If OpenACC is not enabled, this function does nothing.
 */
template <typename T>
void DualMemoryManager::update_array_device_to_host(DualArray<T> dual_array,
                                                    const size_t offset,
                                                    const size_t num_elements) {
  /* check that host pointer is initialized */
  if (dual_array.host_ptr == nullptr)
    abort_mimmo("Host pointer of dual array is a null pointer.");

#ifdef _OPENACC
  /* check that device pointer is initialized */
  if (dual_array.dev_ptr == nullptr)
    abort_mimmo("Device pointer of dual array is a null pointer.");

  /* copy data from device to host */
  acc_memcpy_from_device(dual_array.host_ptr + offset,
                         dual_array.dev_ptr + offset, num_elements * sizeof(T));
#endif

  return;
}

/**
 * @brief Frees memory allocated for a given dual array.
 *
 * @tparam T         Type of elements in the array.
 *
 * @param dual_array Dual array to be freed.
 *
 * @note If the array is not tracked (i.e. was not allocated using this
 *       memory manager, or it was already freed), the program aborts.
 */
template <typename T>
void DualMemoryManager::free_array(DualArray<T> &dual_array) {
  /* check that host pointer is not null */
  if (dual_array.host_ptr == nullptr) {
    abort_mimmo("Host pointer of dual array is a null pointer.");
  }

  /* check that array was actually recorded and update memory
   * tracker
   * */
  const bool ret = remove_from_memory_tracker(memory_tracker, total_memory,
                                              (void *)&dual_array);
  if (ret) {
    abort_mimmo("Dual array was not found by memory manager.");
  }

  /* free memory on host */
  std::free(dual_array.host_ptr);
  dual_array.host_ptr = nullptr;

  /* free memory on device */
#ifdef _OPENACC
  if (dual_array.dev_ptr != nullptr) {
    acc_free(dual_array.dev_ptr);
    dual_array.dev_ptr = nullptr;
  }
#endif // _OPENACC

  return;
}

} // namespace MiMMO
