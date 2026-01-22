/**
 * @file main.cpp
 *
 * @brief Very simple example of scalar product between two arrays performed
 * on device using MiMMO's memory manager.
 */

#include "mimmo/api.hpp"
#include <iostream>
#include <openacc.h>

#define DIM 10

int main() {
  /* instantiate a dual memory manager */
  MiMMO::DualMemoryManager dual_memory_manager = MiMMO::DualMemoryManager();

  /* instantiate dual arrays */
  MiMMO::DualArray<int> dual_array_1 =
      dual_memory_manager.allocate<int>("dual_array_1", DIM, true);
  MiMMO::DualArray<int> dual_array_2 =
      dual_memory_manager.allocate<int>("dual_array_2", DIM, true);
  MiMMO::DualArray<int> dual_array_res =
      dual_memory_manager.allocate<int>("dual_array_res", DIM, true);

  /* print memory usage report */
  dual_memory_manager.report_memory_usage();

  /* initialize host arrays */
  for (int i = 0; i < dual_array_1.dim; i++) {
    dual_array_1.host_ptr[i] = i;
    dual_array_2.host_ptr[i] = 10 * i;
  }

  /* copy needed data to device */
  dual_memory_manager.copy_host_to_device(dual_array_1, 0, dual_array_1.dim);
  dual_memory_manager.copy_host_to_device(dual_array_2, 0, dual_array_2.dim);

  /* OpenACC compute region */
#pragma acc parallel MIMMO_PRESENT(dual_array_1) MIMMO_PRESENT(dual_array_2)   \
    MIMMO_PRESENT(dual_array_res) default(none)
  {
    /* perform calculation on device */
#pragma acc loop
    for (int i = 0; i < dual_array_1.dim; i++)
      MIMMO_GET_PTR(dual_array_res)
    [i] = MIMMO_GET_PTR(dual_array_1)[i] * MIMMO_GET_PTR(dual_array_2)[i];
  }

  /* copy result to host */
  dual_memory_manager.copy_device_to_host(dual_array_res, 0,
                                          dual_array_res.dim);

  /* print results of operation */
  std::cout << "Result array:  [";
  std::cout << dual_array_res.host_ptr[0];
  for (int i = 1; i < dual_array_res.dim; i++)
    std::cout << ",  " << dual_array_res.host_ptr[i];
  std::cout << "]";
  std::cout << std::endl;

  /* free dual array memory */
  dual_memory_manager.free(dual_array_1);
  dual_memory_manager.free(dual_array_2);
  dual_memory_manager.free(dual_array_res);

  return 0;
}
