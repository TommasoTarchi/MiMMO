/**
 * @file api.cpp
 *
 * @brief Main library header with API definitions.
 */

#pragma once

#include "../private/abort_manager.hpp"
#include "../private/memory_tracker.hpp"
#ifdef _OPENACC
#include <openacc.h>
#endif // _OPENACC

/* API */

namespace MiMMO {

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
  T *host_ptr;       /*!< pointer to host memory */
  T *dev_ptr;        /*!< pointer to device memory */
  std::string label; /*!< label for memory tracker */
  size_t dim;        /*!< number of elements in the array */
  size_t size;       /*!< size in bytes of the array */
};

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
   * @param label     Label that should be used to track the array in
   *                  memory.
   * @param dim       Number of elements in the array.
   * @param on_device Whether the array should be allocated on device as
   *                  well.
   *
   * @return Allocated array in the form of an object of type DualArray.
   */
  template <typename T>
  DualArray<T> allocate(const std::string label, const size_t dim,
                        const bool on_device = false);

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
  void copy_host_to_device(DualArray<T> dual_array, const size_t offset,
                           const size_t num_elements);

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
  void copy_device_to_host(DualArray<T> dual_array, const size_t offset,
                           const size_t num_elements);

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
  template <typename T> void free(DualArray<T> &dual_array);

  /**
   * @brief Returns the total host and device memory allocated by
   * the memory manager.
   *
   * @details
   * This function returns a pair where the elements are the total
   * memory allocated on host and device, respectively.
   *
   * @return (total host memory used, total device memory used)
   */
  std::pair<size_t, size_t> return_total_memory_usage();

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

} // namespace MiMMO

/**
 * @brief Returns the right pointer (host or device) depending on whether
 * running on host or device.
 *
 * @details
 * This macro selects the host or the device pointer depending on whether
 * OpenACC is enabled or not.
 *
 * It is thought to be used inside OpenACC compute regions to keep the
 * code clean and avoid #ifdef's.
 *
 * @param x Dual array from which the needed pointer should be selected.
 */
#ifdef _OPENACC
#define MIMMO_GET_PTR(x) x.dev_ptr
#else
#define MIMMO_GET_PTR(x) x.host_ptr
#endif

/**
 * @brief Communicates in an OpenACC pragma that a dual array is present on
 * device and copies its size to device.
 *
 * @details
 * This macro must be used inside an OpenACC pragma placed at the beginning of
 * a compute region to communicate that the dual array is already present on
 * device.
 *
 * Internally, the macro communicates that the device pointer of the dual
 * array is actually a device pointer, and temporarily copies the structure
 * corresponding to the dual array to device.
 *
 * **Notice**: it takes only one argument in input; if you need to use it on
 * multiple arrays within the same pragma, use multiple calls to the macro.
 *
 * **Notice**: there is no corresponding macro for copy on purpose, since all
 * data movements of dual arrays are expected to be performed using methods of
 * DualMemoryManager.
 *
 * @param x Dual array present on device.
 */
#ifdef _OPENACC
#define MIMMO_PRESENT(x) copy(x) deviceptr(x.dev_ptr)
#else
#define MIMMO_PRESENT(x) ()
#endif

/* include of templated methods definitions */

#include "../private/allocate.inl"
#include "../private/copy.inl"
#include "../private/free.inl"
