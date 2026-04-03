/**
 * @file main.cpp
 *
 * @brief Advanced example: scalar product using an OpenACC routine.
 *
 * Similar to examples/scalar_product/plain/main.cpp, but demonstrates:
 * - Calling a subroutine from within an OpenACC parallel region
 * - Using templates with #pragma acc routine
 *
 * @ingroup examples
 *
 * @see DualMemoryManager::alloc_array()
 * @see MIMMO_GET_PTR
 * @see MIMMO_PRESENT
 */

#include "mimmo/api.hpp"
#include <iostream>
#include <openacc.h>

#define DIM 10

/* subroutine for scalar product evaluation (done sequentially
 * for simplicity)
 * */
#pragma acc routine
template <typename T>
void scalar_product(const MiMMO::DualArray<T> dual_array_1,
                    const MiMMO::DualArray<T> dual_array_2,
                    const MiMMO::DualArray<T> dual_array_res) {

#pragma acc loop
  for (size_t i = 0; i < dual_array_1.size; i++)
    MIMMO_GET_PTR(dual_array_res)
  [i] = MIMMO_GET_PTR(dual_array_1)[i] * MIMMO_GET_PTR(dual_array_2)[i];

  return;
}

int main() {
  /* instantiate a dual memory manager */
  MiMMO::DualMemoryManager dual_memory_manager = MiMMO::DualMemoryManager();

  /* instantiate dual arrays */
  MiMMO::DualArray<int> dual_array_1;
  dual_memory_manager.alloc_array(dual_array_1, "dual_array_1", DIM, true);
  MiMMO::DualArray<int> dual_array_2;
  dual_memory_manager.alloc_array(dual_array_2, "dual_array_2", DIM, true);
  MiMMO::DualArray<int> dual_array_res;
  dual_memory_manager.alloc_array(dual_array_res, "dual_array_res", DIM, true);

  /* print memory usage report */
  dual_memory_manager.report_memory_usage();

  /* initialize host arrays */
  for (int i = 0; i < dual_array_1.size; i++) {
    dual_array_1.host_ptr[i] = i;
    dual_array_2.host_ptr[i] = 10 * i;
  }

  /* copy needed data to device */
  dual_memory_manager.update_array_host_to_device(dual_array_1, 0,
                                                  dual_array_1.size);
  dual_memory_manager.update_array_host_to_device(dual_array_2, 0,
                                                  dual_array_2.size);

  /* OpenACC compute region */
#pragma acc parallel MIMMO_PRESENT(dual_array_1) MIMMO_PRESENT(dual_array_2)   \
    MIMMO_PRESENT(dual_array_res) default(none)
  {
    /* perform calculation on device */
    scalar_product(dual_array_1, dual_array_2, dual_array_res);
  }

  /* copy result to host */
  dual_memory_manager.update_array_device_to_host(dual_array_res, 0,
                                                  dual_array_res.size);

  /* print results of operation */
  std::cout << "Result array:  [";
  std::cout << dual_array_res.host_ptr[0];
  for (int i = 1; i < dual_array_res.size; i++)
    std::cout << ",  " << dual_array_res.host_ptr[i];
  std::cout << "]";
  std::cout << std::endl;

  /* free dual array memory */
  dual_memory_manager.free_array(dual_array_1);
  dual_memory_manager.free_array(dual_array_2);
  dual_memory_manager.free_array(dual_array_res);

  return 0;
}
