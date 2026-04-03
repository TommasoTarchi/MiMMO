/**
 * @file api.hpp
 *
 * @brief Main library header with API definitions.
 *
 * @mainpage MiMMO (Minimal Memory Manager for OpenACC)
 *
 * This is the main public header of the MiMMO library. It provides:
 * - DualArray and DualScalar data structures for host/device memory
 * - DualMemoryManager class for memory operations
 * - Helper macros for OpenACC compute regions
 *
 * @section example_usage Example Usage
 *
 * @code
 * #include "mimmo/api.hpp"
 *
 * MiMMO::DualMemoryManager manager;
 * MiMMO::DualArray<int> array;
 *
 * manager.alloc_array(array, "my_array", 100, true);
 *
 * // ... initialize data on host ...
 *
 * manager.update_array_host_to_device(array, 0, array.size);
 *
 * #pragma acc parallel MIMMO_PRESENT(array) default(none)
 * {
 *     // Use MIMMO_GET_PTR(array) inside compute regions
 * }
 *
 * manager.update_array_device_to_host(array, 0, array.size);
 * manager.free_array(array);
 * @endcode
 *
 * @see examples/scalar_product/plain/main.cpp
 * @see examples/globals/main.cpp
 */

#pragma once

#include "../private/abort.hpp"
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
  size_t size;       /*!< number of elements in the array */
  size_t size_bytes; /*!< size in bytes of the array */
};

/**
 * @brief Stores dual scalar data.
 *
 * @details
 * This struct contains all needed information related to a dual
 * scalar, i.e. the couple host value-device pointer.
 *
 * @tparam T Type of scalar variable.
 */
template <typename T> struct DualScalar {
  T host_value; /*!< value on host */
  T *dev_ptr;   /*!< pointer to value on device */
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
  std::pair<size_t, size_t> total_memory; /*!< total used memory on host
                                               and device */
  std::map<void *, std::tuple<std::string, size_t, bool>>
      memory_tracker; /*!< memory tracker for reports */

public:
  /**
   * @brief Class constructor.
   */
  DualMemoryManager() : total_memory({0, 0}), memory_tracker({}) {}

  /**
   * @brief Allocates dual array memory.
   *
   * @details
   * This function allocates memory on host and (if requested) on device
   * for a certain array. It returns an object of type DualArray,
   * containing host and device pointers.
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
  void alloc_array(DualArray<T> &dual_array, const std::string label,
                   const size_t size, const bool on_device = false);

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
  void update_array_host_to_device(DualArray<T> dual_array, const size_t offset,
                                   const size_t num_elements);

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
  void update_array_device_to_host(DualArray<T> dual_array, const size_t offset,
                                   const size_t num_elements);

  /**
   * @brief Frees memory allocated for a given dual array.
   *
   * @tparam T         Type of elements in the array.
   *
   * @param dual_array Dual array to be freed.
   *
   * @note If the array is not tracked, the program aborts.
   */
  template <typename T> void free_array(DualArray<T> &dual_array);

  /**
   * @brief Creates a dual scalar.
   *
   * @details
   * This function creates a scalar variable on host and optionally on
   * device (allocating a one-element array), and assigns the requested
   * value. It returns an object of type DualScalar that contains the host
   * value and the device pointer.
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
  void create_scalar(DualScalar<T> &dual_scalar, const std::string label,
                     const T value, const bool on_device = false);

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
  void update_scalar_host_to_device(DualScalar<T> &dual_scalar);

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
  void update_scalar_device_to_host(DualScalar<T> &dual_scalar);

  /**
   * @brief Frees memory allocated on device for a given scalar.
   *
   * @tparam T          Type of the scalar variable.
   *
   * @param dual_scalar Dual scalar to be destroyed.
   *
   * @note If the scalar is not tracked, the program aborts.
   *       If OpenACC is not enabled, this function does nothing.
   */
  template <typename T> void destroy_scalar(DualScalar<T> &dual_scalar);

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

  /// @todo Consider adding a destructor to clean up tracked memory.
};

} // namespace MiMMO

/**
 * @brief Returns the device or host pointer depending on compilation flags.
 *
 * @param x Dual array from which the pointer should be selected.
 *
 * @note Use inside OpenACC compute regions only.
 */
#ifdef _OPENACC
#define MIMMO_GET_PTR(x) (x).dev_ptr
#else
#define MIMMO_GET_PTR(x) (x).host_ptr
#endif // _OPENACC

/**
 * @brief Returns the device or host value depending on compilation flags.
 *
 * @param x Dual scalar from which the value should be selected.
 *
 * @note Use inside OpenACC compute regions only.
 */
#ifdef _OPENACC
#define MIMMO_GET_VALUE(x) *((x).dev_ptr)
#else
#define MIMMO_GET_VALUE(x) (x).host_value
#endif // _OPENACC

/**
 * @brief Communicates in an OpenACC pragma that a dual array or scalar is
 * present on device.
 *
 * @param x Dual array or scalar present on device.
 *
 * @note Must be used inside an OpenACC pragma at the beginning of a compute
 *       region.
 * @note Takes only one argument; use multiple calls for multiple objects.
 * @note No copy variant exists by design; use DualMemoryManager methods for
 *       data movement.
 */
#ifdef _OPENACC
#define MIMMO_PRESENT(x) copyin(x) deviceptr(x.dev_ptr)
#else
#define MIMMO_PRESENT(x)
#endif // _OPENACC

/* include of templated methods definitions */

#include "../private/arrays.inl"
#include "../private/scalars.inl"
