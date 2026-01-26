/**
 * @file allocate.inl
 *
 * @brief Definition of method for managing dual scalars.
 */

#pragma once

namespace MiMMO {

// todo: add memory tracking
// todo: add description
template <typename T>
DualScalar<T> create_scalar(const std::string label, const T value,
                            const bool on_device) {
  DualScalar<T> dual_scalar;

  /* define value on host */
  dual_scalar.host_value = value;

  /* if required, allocate memory on device */
#ifdef _openacc
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
#endif // _openacc

  /* copy data from host to device */
#ifdef _OPENACC
  acc_memcpy_to_device(dual_scalar.dev_ptr, &(dual_scalar.host_value),
                       sizeof(T));
#endif // _OPENACC

  /* update scalar label */
  dual_scalar.label = label;

  return dual_scalar;
}

// TODO: add description
template <typename T>
void update_scalar_value(DualScalar<T> &dual_scalar, const int value) {
  /* update host value */
  dual_scalar.host_value = value;

  /* copy data from host to device */
#ifdef _OPENACC
  acc_memcpy_to_device(dual_scalar.dev_ptr, &(dual_scalar.host_value),
                       sizeof(T));
#endif // _OPENACC

  return;
}

// TODO: add memory tracking
// TODO: add description
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
