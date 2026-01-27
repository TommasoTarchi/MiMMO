/**
 * @file allocate.inl
 *
 * @brief Definition of method for managing dual scalars.
 */

#pragma once

namespace MiMMO {

// TODO: add memory tracking
/**
 * @brief Creates a dual scalar.
 *
 * @details
 * This function creates a scalar variable on host and optionally on
 * device (allocating a one-element array), and assigns the requested
 * value. It returns an object of type DualScalar that contains the host
 * value and the device pointer.
 *
 * @tparam T Type of element of variable to be created.
 *
 * @param label     Label that should be used to track the scalar in
 *                  memory.
 * @param value     Value to which the scalar should be initialized.
 * @param on_device Whether the scalar should be created on device as
 *                  well (ignored if main code compiled without OpenACC
 *                  support).
 *
 * @return Created variable in the form of an object of type DualScalar.
 */
template <typename T>
DualScalar<T> DualMemoryManager::create_scalar(const std::string label,
                                               const T value,
                                               const bool on_device) {
  DualScalar<T> dual_scalar;

  /* define value on host */
  dual_scalar.host_value = value;

  /* if required, allocate memory on device */
#ifdef _OPENACC
  if (on_device) {
    dual_scalar.dev_ptr = (T *)acc_malloc(sizeof(T));

    if (!(dual_scalar.dev_ptr)) {
      abort_manager("Failed to allocate device memory.");
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

  /* update scalar label */
  dual_scalar.label = label;

  return dual_scalar;
}

/**
 * @brief Sets a dual scalar value.
 *
 * @details
 * This function updates the value of a dual scalar on host **or** on
 * device.
 *
 * **Warning**: the scalar must have been previously created using the
 * create_scalar() method.
 *
 * @param dual_scalar  Dual scalar to be updated.
 * @param value        Value to which the scalar should be set.
 * @param on_device    'true' if the scalar should be updated on device,
 *                     'false' if it should be updated on host (if 'true'
 *                     and OpenACC is not enabled, the function does
 *                     nothing).
 */
template <typename T>
void DualMemoryManager::set_scalar_value(DualScalar<T> &dual_scalar,
                                         const T value, const bool on_device) {
  if (on_device) {

#ifdef _OPENACC
    /* check that device pointer is initialized */
    if (dual_scalar.dev_ptr == nullptr)
      abort_manager(dual_scalar.label + "'s device pointer is a null pointer.");

    /* copy data from host to device */
    T value_tmp = value;
    acc_memcpy_to_device(dual_scalar.dev_ptr, &value_tmp, sizeof(T));
#endif // _OPENACC

  } else {

    /* update host value */
    dual_scalar.host_value = value;
  }

  return;
}

// TODO: add memory tracking
/**
 * @brief Frees memory allocated on device for a given scalar.
 *
 * @details
 * This function frees memory allocated on device for a given dual
 * scalar.
 *
 * If the array is not tracked (i.e. was not allocated using this
 * memory manager, or it was already freed), the program aborts.
 *
 * If OpenACC is not enabled, this function does nothing.
 *
 * @param dual_scalar Dual scalar to be destroyed.
 */
template <typename T>
void DualMemoryManager::destroy_scalar(DualScalar<T> &dual_scalar) {

  /* free memory on device */
  if (dual_scalar.dev_ptr != nullptr) {
#ifdef _OPENACC
    acc_free(dual_scalar.dev_ptr);
#endif // _OPENACC
    dual_scalar.dev_ptr = nullptr;
  }

  return;
}

} // namespace MiMMO
