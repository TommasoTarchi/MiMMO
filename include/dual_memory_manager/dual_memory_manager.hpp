// TODO: add file description

#pragma once

#include "../private/abort_manager.hpp"
#include "../private/memory_tracker.hpp"
#ifdef _OPENACC
#include <openacc.h>
#endif // _OPENACC

namespace DualMemoryManager {

// TODO: add description and comments
template <typename T> struct DualArray {
  T *host_ptr;
  T *dev_ptr;
  std::string label;
  size_t num_elements;
  size_t size;
};

// TODO: consider if instead of ifdef's it would be better to template
//       the class over a bool telling whether GPU is used or not
// TODO: separate methods definitions from their declarations (use .incl
//       files if needed)
// TODO: add description
// TODO: maye add alignment (either to the constructor or to the memory
//       allocation call
class DualMemoryManager {
private:
  size_t total_host_memory;   /*!< total used memory on host */
  size_t total_device_memory; /*!< total used memory on device */
  std::map<std::string, size_t> host_memory_tracker;   /*!< host memory
                                                        tracker for reports */
  std::map<std::string, size_t> device_memory_tracker; /*!< device memory
                                                         tracker for reports */

public:
  DualMemoryManager()
      : total_host_memory(0), total_device_memory(0), host_memory_tracker({}),
        device_memory_tracker({}) {}

  // TODO: add description
  template <typename T>
  DualArray<T> allocate(const std::string label, const size_t num_elements,
                        const bool on_device = false) {
    DualArray<T> dual_array;

    /* allocate memory on host */
    dual_array.host_ptr = (T *)std::malloc(num_elements * sizeof(T));

#ifdef _OPENACC
    /* if required, allocate memory on device */
    if (on_device) {
      dual_array.dev_ptr = (T *)acc_malloc(num_elements * sizeof(T));

      if (dual_array.dev_ptr == nullptr) {
        std::free(dual_array.host_ptr);
        dual_array.host_ptr = nullptr;
        abort_manager("Failed to allocate device memory.");
      }
    } else {
      dual_array.dev_ptr = nullptr;
    }
#else
    dual_array.dev_ptr = nullptr;
#endif // _OPENACC

    /* update array label */
    dual_array.label = label;

    /* update number of elements and bytes */
    dual_array.num_elements = num_elements;
    dual_array.size = num_elements * sizeof(T);

    /* update used memory */
    total_host_memory += dual_array.size;
    const bool ret_host =
        add_to_memory_tracker(host_memory_tracker, label, dual_array.size);

    bool ret_device = false;
#ifdef _OPENACC
    if (on_device) {
      total_device_memory += dual_array.size;
      ret_device =
          add_to_memory_tracker(device_memory_tracker, label, dual_array.size);
    }
#endif // _OPENACC

    if (ret_host || ret_device)
      abort_manager(label + " already exists. Please choose another label.");

    return dual_array;
  }

  // TODO: add description (specify when the method aborts)
  template <typename T> void free(DualArray<T> &dual_array) {
    /* check that host pointer is not null */
    if (dual_array.host_ptr == nullptr) {
      abort_manager(dual_array.label + "'s host pointer is a null pointer.");
    }

    /* update host memory usage */
    bool ret =
        remove_from_memory_tracker(host_memory_tracker, dual_array.label);
    if (ret) {
      abort_manager(dual_array.label +
                    " was not found by memory manager on host.");
    }
    total_host_memory -= dual_array.size;

    /* free memory on host */
    std::free(dual_array.host_ptr);
    dual_array.host_ptr = nullptr;

#ifdef _OPENACC
    if (dual_array.dev_ptr != nullptr) {
      /* free memory on device */
      acc_free(dual_array.dev_ptr);
      dual_array.dev_ptr = nullptr;

      /* update device memory usage*/
      ret = remove_from_memory_tracker(device_memory_tracker, dual_array.label);
      if (ret) {
        abort_manager(dual_array.label +
                      " was not found by memory manager on device, but the "
                      "corresponding device pointer is not a null pointer.");
      }
      total_device_memory -= dual_array.size;
    }
#endif // _OPENACC
  }

  // TODO: add description
  void report_memory_usage();

  // TODO: maybe the destructor is needed?
};

} // namespace DualMemoryManager
