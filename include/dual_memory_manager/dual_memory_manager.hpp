#include <iostream>
#include <openacc.h>

namespace DualMemoryManager {

template <typename T> struct DualMemory {
  T *host_ptr;
  T *dev_ptr;
  size_t num_elements;
  size_t size;
};

// TODO: maye add alignment (either to the constructor or to the memory
//       allocation call
class DualMemoryManager {
public:
  DualMemoryManager(void) {}

  template <typename T>
  DualMemory<T> allocate(const size_t num_elements,
                         const bool gpu_enabled = false) {
    DualMemory<T> dual_array;

    dual_array.host_ptr = (T *)malloc(num_elements * sizeof(T));

    if (gpu_enabled) {
      dual_array.dev_ptr = (T *)acc_malloc(num_elements * sizeof(T));

      if (dual_array.dev_ptr == nullptr) {
        std::cerr << "DualMemoryManager error: failed to allocate device memory."
                  << std::endl;
        exit(1);
      }
    } else {
      dual_array.dev_ptr = nullptr;
    }

    dual_array.num_elements = num_elements;
    dual_array.size = num_elements * sizeof(T);
  }

  template <typename T> void free(DualMemory<T> dual_array) {
    free(dual_array.host_ptr);
    dual_array.host_ptr = nullptr;

    if (dual_array.dev_ptr != nullptr) {
      acc_free(dual_array.dev_ptr);
      dual_array.dev_ptr = nullptr;
    }
  }
};

} // namespace DualMemoryManager
