/**
 * @file scalars.inl
 *
 * @brief Definition of template methods for managing dual scalars.
 *
 * Implements the following DualMemoryManager methods:
 * - create_scalar()
 * - update_scalar_host_to_device()
 * - update_scalar_device_to_host()
 * - destroy_scalar()
 *
 * @see api.hpp for the corresponding declarations
 * @see examples/globals/main.cpp for usage with global variables
 */

#pragma once

namespace MiMMO {

/**
 * @brief Creates a dual scalar.
 *
 * @tparam T          Type of the scalar variable.
 *
 * @param dual_scalar Dual scalar to be created.
 * @param label       Label that should be used to track the scalar in
 *                    memory.
 * @param value       Value to which the scalar should be initialized.
 * @param on_device   Whether the scalar should be created on device as
 *                    well (ignored if main code compiled without OpenACC
 *                    support).
 */
template <typename T>
void DualMemoryManager::create_scalar(DualScalar<T> &dual_scalar,
                                      const std::string label, const T value,
                                      const bool on_device) {

  /* define value on host */
  dual_scalar.host_value = value;

  /* if required, allocate memory on device */
#ifdef _OPENACC
  if (on_device) {
    dual_scalar.dev_ptr = (T *)acc_malloc(sizeof(T));

    if (!(dual_scalar.dev_ptr)) {
      abort_mimmo("Failed to allocate device memory.");
    }

  } else {
    dual_scalar.dev_ptr = nullptr;
  }
#else
  dual_scalar.dev_ptr = nullptr;
#endif // _OPENACC

  /* copy data from host to device */
#ifdef _OPENACC
  if (on_device)
    acc_memcpy_to_device(dual_scalar.dev_ptr, &(dual_scalar.host_value),
                         sizeof(T));
#endif // _OPENACC

  /* update memory tracker */
#ifdef _OPENACC
  const bool ret =
      add_to_memory_tracker(memory_tracker, total_memory, (void *)&dual_scalar,
                            label, sizeof(T), on_device);
#else
  const bool ret =
      add_to_memory_tracker(memory_tracker, total_memory, (void *)&dual_scalar,
                            label, sizeof(T), false);
#endif // _OPENACC

  if (ret)
    abort_mimmo("Failed to track memory for dual scalar '" + label + "'.");

  return;
}

/**
 * @brief Updates the value of a dual scalar from host to device.
 *
 * @tparam T          Type of the scalar variable.
 *
 * @param dual_scalar Dual scalar to synchronize.
 *
 * @note The scalar must have been previously created using create_scalar().
 *       If the scalar is not present on device, the program aborts.
 */
template <typename T>
void DualMemoryManager::update_scalar_host_to_device(
    DualScalar<T> &dual_scalar) {

#ifdef _OPENACC
  /* check that device pointer is initialized */
  if (dual_scalar.dev_ptr == nullptr)
    abort_mimmo("Device pointer of dual scalar is a null pointer.");

  /* copy data from host to device */
  acc_memcpy_to_device(dual_scalar.dev_ptr, &dual_scalar.host_value, sizeof(T));
#endif // _OPENACC

  return;
}

/**
 * @brief Updates the value of a dual scalar from device to host.
 *
 * @tparam T          Type of the scalar variable.
 *
 * @param dual_scalar Dual scalar to synchronize.
 *
 * @note The scalar must have been previously created using create_scalar().
 *       If the scalar is not present on device, the program aborts.
 *       If OpenACC is not enabled, this function does nothing.
 */
template <typename T>
void DualMemoryManager::update_scalar_device_to_host(
    DualScalar<T> &dual_scalar) {

#ifdef _OPENACC
  /* check that device pointer is initialized */
  if (dual_scalar.dev_ptr == nullptr)
    abort_mimmo("Device pointer of dual scalar is a null pointer.");

  /* copy data from device to host */
  acc_memcpy_from_device(&dual_scalar.host_value, dual_scalar.dev_ptr,
                         sizeof(T));
#endif // _OPENACC

  return;
}

/**
 * @brief Frees memory allocated on device for a given scalar.
 *
 * @tparam T          Type of the scalar variable.
 *
 * @param dual_scalar Dual scalar to be destroyed.
 *
 * @note If the scalar is not tracked (i.e. was not allocated using this
 *       memory manager, or it was already freed), the program aborts.
 *       If OpenACC is not enabled, this function does nothing.
 */
template <typename T>
void DualMemoryManager::destroy_scalar(DualScalar<T> &dual_scalar) {

  /* check that scalar was actually recorded and update memory
   * tracker
   * */
  const bool ret = remove_from_memory_tracker(memory_tracker, total_memory,
                                              (void *)&dual_scalar);
  if (ret) {
    abort_mimmo("Dual scalar was not found by memory manager.");
  }

  /* free memory on device */
#ifdef _OPENACC
  if (dual_scalar.dev_ptr != nullptr) {
    acc_free(dual_scalar.dev_ptr);
    dual_scalar.dev_ptr = nullptr;
  }
#endif // _OPENACC

  return;
}

} // namespace MiMMO
