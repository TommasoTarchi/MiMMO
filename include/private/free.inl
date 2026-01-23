/**
 * @file free.inl
 *
 * @brief Definition of method for freeing dual arrays.
 */

#pragma once

namespace MiMMO {

/**
 * @brief Frees memory allocated for a given dual array.
 *
 * @details
 * This function frees memory allocated for a given dual array.
 *
 * If the array is not tracked (i.e. was not allocated using this
 * memory manager, or it was already freed), the program aborts.
 *
 * @param dual_array Dual array to be freed.
 */
template <typename T> void DualMemoryManager::free(DualArray<T> &dual_array) {
  /* check that host pointer is not null */
  if (dual_array.host_ptr == nullptr) {
    abort_manager(dual_array.label + "'s host pointer is a null pointer.");
  }

  /* check that array was actually recorded and update memory
   * tracker
   * */
  bool ret = remove_from_memory_tracker(memory_tracker, dual_array.label);
  if (ret) {
    abort_manager(dual_array.label + " was not found by memory manager.");
  }

  /* free memory on host */
  std::free(dual_array.host_ptr);
  dual_array.host_ptr = nullptr;
  total_host_memory -= dual_array.size_bytes;

  /* free memory on device */
  if (dual_array.dev_ptr != nullptr) {
#ifdef _OPENACC
    acc_free(dual_array.dev_ptr);
#endif // _OPENACC
    dual_array.dev_ptr = nullptr;
    total_device_memory -= dual_array.size_bytes;
  }
}

} // namespace MiMMO
