/**
 * @file dual_memory_manager.cpp
 *
 * @brief Main library header with memory manager class definition.
 */

#pragma once

#include "../private/abort_manager.hpp"
#include "../private/memory_tracker.hpp"
#ifdef _OPENACC
#include <openacc.h>
#endif // _OPENACC

namespace DualMemoryManager {

/**
 * @brief Stores dual array data.
 *
 * @details
 * This struct contains all needed information related to a dual
 * array, i.e. the couple host pointer-device pointer.
 *
 * @tparam T Type of elements in the array.
 */
template <typename T> struct DualArray {
  T *host_ptr;         /*!< pointer to host memory */
  T *dev_ptr;          /*!< pointer to device memory */
  std::string label;   /*!< label for memory tracker */
  size_t num_elements; /*!< number of elements in the array */
  size_t size;         /*!< size in bytes of the array */
};

// TODO: add methods for copying data host-to-device and vice versa
// TODO: separate methods definitions from their declarations (use .incl
//       files if needed)
// TODO: maye add alignment (either to the constructor or to the memory
//       allocation call
/**
 * @brief Class for host-device memory management.
 *
 * @details
 * This class can be used to manage host and device memory, using objects
 * of type DualArray.
 *
 * The memory manager keeps track of allocated arrays on host and device
 * using its tracker.
 */
class DualMemoryManager {
private:
  size_t total_host_memory;   /*!< total used memory on host */
  size_t total_device_memory; /*!< total used memory on device */
  std::map<std::string, std::pair<size_t, bool>>
      memory_tracker; /*!< memory tracker for reports */

public:
  /**
   * @brief Class constructor.
   */
  DualMemoryManager()
      : total_host_memory(0), total_device_memory(0), memory_tracker({}) {}

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
  DualArray<T> allocate(const std::string label, const size_t num_elements,
                        const bool on_device = false) {
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

    const bool ret = add_to_memory_tracker(memory_tracker, label,
                                           dual_array.size, on_device);

    if (ret)
      abort_manager(label + " already exists. Please choose another label.");

    return dual_array;
  }

  /**
   * @brief Frees memory allocated for a given dual array.
   *
   * @details
   * This function frees memory allocated for a given dual array.
   *
   * If the array is not tracked (i.e. was not allocated using this
   * memory manager, or it was already freed), the program aborts.
   *
   * @tparam T Type of elements in array to be freed.
   *
   * @param dual_array Dual array to be freed.
   */
  template <typename T> void free(DualArray<T> &dual_array) {
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
    total_host_memory -= dual_array.size;

    /* free memory on device */
    if (dual_array.dev_ptr != nullptr) {
#ifdef _OPENACC
      acc_free(dual_array.dev_ptr);
#endif // _OPENACC
      dual_array.dev_ptr = nullptr;
      total_device_memory -= dual_array.size;
    }
  }

  /**
   * @brief Reports memory used by the memory manager.
   *
   * @details
   * This function prints to standard output a complete report of memory
   * usage of the memory manager.
   *
   * A list of all allocated arrays is shown, with size (in bytes) and
   * whether the array is present on device or not.
   */
  void report_memory_usage();

  // TODO: maybe the destructor is needed?
};

} // namespace DualMemoryManager
