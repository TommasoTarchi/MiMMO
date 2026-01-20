/**
 * @file main.cpp
 *
 * @brief Simple example of global variables manipulation using MiMMO
 * library.
 */

#include "allvars.hpp"
#include <iostream>

int main() {
  /* instantiate the dual memory manager */
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  /* define size of global array */
  global_array_size = 10;

  /* allocate memory for global array */
  global_array =
      memory_manager.allocate<int>("global_array", global_array_size, true);

  /* initialize array on host */
  for (int i = 0; i < global_array_size; i++)
    global_array.host_ptr[i] = i;

  /* copy array to device */
  memory_manager.copy_host_to_device(global_array);

  /* perform calculation on device */
#pragma acc parallel MIMMO_PRESENT(global_array) default(none)
  {
#pragma acc loop
    for (int i = 0; i < MIMMO_GET_DIM(global_array); i++)
      MIMMO_GET_PTR(global_array)[i] *= 10;
  }

  /* copy data back to host */
  memory_manager.copy_device_to_host(global_array);

  /* print final results */
  std::cout << "Result array:  [";
  std::cout << global_array.host_ptr[0];
  for (int i = 1; i < MIMMO_GET_DIM(global_array); i++)
    std::cout << ",  " << global_array.host_ptr[i];
  std::cout << "]";
  std::cout << std::endl;

  /* free global array */
  memory_manager.free(global_array);

  return 0;
}
