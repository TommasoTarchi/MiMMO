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
      memory_manager.alloc_array<int>("global_array", global_array_size, true);

  /* initialize array on host */
  for (int i = 0; i < global_array_size; i++)
    global_array.host_ptr[i] = i;

  /* copy array to device */
  memory_manager.update_array_host_to_device(global_array, 0,
                                             global_array.size);

  /* create scalar */
  global_scalar = memory_manager.create_scalar<int>("global_scalar", 10, true);

  /* perform calculation on device */
#pragma acc parallel MIMMO_PRESENT(global_array)                               \
    MIMMO_PRESENT(global_scalar) default(none)
  {
#pragma acc loop
    for (int i = 0; i < global_array.size; i++)
      MIMMO_GET_PTR(global_array)[i] *= MIMMO_GET_VALUE(global_scalar);

    MIMMO_GET_VALUE(global_scalar) += 5;
  }

  /* copy data back to host */
  memory_manager.update_array_device_to_host(global_array, 0,
                                             global_array.size);
  memory_manager.update_scalar_device_to_host(global_scalar);

  /* print final results */
  std::cout << "Result array:  [";
  std::cout << global_array.host_ptr[0];
  for (int i = 1; i < global_array.size; i++)
    std::cout << ",  " << global_array.host_ptr[i];
  std::cout << "]\n" << "Result scalar: " << global_scalar.host_value;
  std::cout << std::endl;

  /* free global array and destroy global scalar */
  memory_manager.free_array(global_array);
  memory_manager.destroy_scalar(global_scalar);

  return 0;
}
