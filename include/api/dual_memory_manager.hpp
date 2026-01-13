#include <iostream>
#include <openacc.h>

namespace DualMemoryManager {

// TODO: add description and comments
template <typename T> struct DualMemory {
  T *host_ptr;
  T *dev_ptr;
  size_t num_elements;
  size_t size;
};

// TODO: add description
// TODO: maye add alignment (either to the constructor or to the memory
//       allocation call
class DualMemoryManager {
private:
  size_t total_host_memory;  /*!< total used memory on host */
  size_t total_device_memory;  /*!< total used memory on device */

public:
  DualMemoryManager(void) : total_host_memory(0), total_device_memory(0) {}

  template <typename T>
  DualMemory<T> allocate(const std::string label, const size_t num_elements,
                         const bool on_device = false) {
    DualMemory<T> dual_array;

    /* allocate memory on host */
    dual_array.host_ptr = (T *)std::malloc(num_elements * sizeof(T));

    /* if required, allocate memory on device */
    if (on_device) {
      dual_array.dev_ptr = (T *)acc_malloc(num_elements * sizeof(T));

      if (dual_array.dev_ptr == nullptr) {
        std::cerr
            << "DualMemoryManager error: failed to allocate device memory."
            << std::endl;
        std::free(dual_array.host_ptr);
        std::exit(1);
      }
    } else {
      dual_array.dev_ptr = nullptr;
    }

    /* update number of elements and bytes */
    dual_array.num_elements = num_elements;
    dual_array.size = num_elements * sizeof(T);

    /* update used memory */
    total_host_memory += dual_array.size;
    if (on_device)
      total_device_memory += dual_array.size;

    return dual_array;
  }

  template <typename T> void free(DualMemory<T> &dual_array) {
    /* free memory on host */
    if (dual_array.host_ptr != nullptr) {
      std::free(dual_array.host_ptr);
      dual_array.host_ptr = nullptr;
      total_host_memory -= dual_array.size;
    }

    /* free memory on device (if present) */
    if (dual_array.dev_ptr != nullptr) {
      acc_free(dual_array.dev_ptr);
      dual_array.dev_ptr = nullptr;
      total_device_memory -= dual_array.size;
    }
  }
};

} // namespace DualMemoryManager
