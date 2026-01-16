/**
 * @file unit_tests_main.cpp
 *
 * @brief Unit tests for the MiMMO library, using Catch2.
 */

#include "../include/mimmo/api.hpp"
#include <catch2/catch_test_macros.hpp>

/**
 * @brief Simple struct for library testing.
 */
struct test_struct {
  double first_field;
  int second_field;
};

/**
 * @brief Memory manager test using basic types ('int' and 'float') without GPU
 * support.
 */
TEST_CASE("Memory manager - base types - No device", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<int> first_test_array =
      memory_manager.allocate<int>("first_test_array", 10, false);

  memory_manager.report_memory_usage();

  MiMMO::DualArray<float> second_test_array =
      memory_manager.allocate<float>("second_test_array", 20, false);

  memory_manager.report_memory_usage();

  memory_manager.free(first_test_array);

  memory_manager.report_memory_usage();

  memory_manager.free(second_test_array);

  memory_manager.report_memory_usage();

  REQUIRE(true);
}

/**
 * @brief Memory manager test using test struct without GPU support.
 */
TEST_CASE("Memory manager - struct - No device", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<test_struct> test_array =
      memory_manager.allocate<test_struct>("test_array", 10, false);

  memory_manager.report_memory_usage();

  memory_manager.free(test_array);

  memory_manager.report_memory_usage();

  REQUIRE(true);
}

/**
 * @brief Pointer selection macro test without GPU support.
 */
TEST_CASE("Pointer selection - No device", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<int> test_array =
      memory_manager.allocate<int>("first_test_array", 10, false);

  const int *ref_ptr = test_array.host_ptr;
  const int *test_ptr = MIMMO_GET_PTR(test_array);

  REQUIRE(test_ptr == ref_ptr);

  memory_manager.free(test_array);
}

/**
 * @brief Size retrieving macro test without GPU support.
 */
TEST_CASE("Size retrieving", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<int> test_array =
      memory_manager.allocate<int>("first_test_array", 10, false);

  const size_t ref_dim = test_array.dim;
  const size_t test_dim = MIMMO_GET_DIM(test_array);

  REQUIRE(test_dim == ref_dim);

  memory_manager.free(test_array);
}

#ifdef _OPENACC
/**
 * @brief Memory manager test using basic types ('int' and 'float') with GPU
 * support.
 */
TEST_CASE("Memory manager - base types", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<int> first_test_array =
      memory_manager.allocate<int>("first_test_array", 10, true);

  memory_manager.report_memory_usage();

  MiMMO::DualArray<float> second_test_array =
      memory_manager.allocate<float>("second_test_array", 20, false);

  memory_manager.report_memory_usage();

  memory_manager.free(first_test_array);

  memory_manager.report_memory_usage();

  memory_manager.free(second_test_array);

  memory_manager.report_memory_usage();

  REQUIRE(true);
}

/**
 * @brief Memory manager test using test struct with GPU support.
 */
TEST_CASE("Memory manager - struct", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<test_struct> test_array =
      memory_manager.allocate<test_struct>("test_array", 10, true);

  memory_manager.report_memory_usage();

  memory_manager.free(test_array);

  memory_manager.report_memory_usage();

  REQUIRE(true);
}

/**
 * @brief Pointer selection macro test with GPU support.
 */
TEST_CASE("Pointer selection", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<int> test_array =
      memory_manager.allocate<int>("first_test_array", 10, true);

  const int *ref_ptr_dev = test_array.dev_ptr;
  const int *ref_ptr_host = test_array.host_ptr;
  const int *test_ptr = MIMMO_GET_PTR(test_array);

  REQUIRE(test_ptr == ref_ptr_dev && test_ptr != ref_ptr_host);

  memory_manager.free(test_array);
}

/**
 * @brief Memory movements test (host-to-device and device-to-host).
 */
TEST_CASE("Memcopy", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<int> test_array =
      memory_manager.allocate<int>("first_test_array", 5, true);

  for (int i = 0; i < 5; i++)
    test_array.host_ptr[i] = i;

  memory_manager.copy_host_to_device(test_array);

#pragma acc parallel deviceptr(test_array.dev_ptr)
  {
#pragma acc loop
    for (int i = 0; i < 5; i++)
      test_array.dev_ptr[i] *= 10;
  }

  memory_manager.copy_device_to_host(test_array);

  REQUIRE((test_array.dev_ptr[0] == test_array.host_ptr[0] * 10) &&
          (test_array.dev_ptr[1] == test_array.host_ptr[1] * 10) &&
          (test_array.dev_ptr[2] == test_array.host_ptr[2] * 10) &&
          (test_array.dev_ptr[3] == test_array.host_ptr[3] * 10) &&
          (test_array.dev_ptr[4] == test_array.host_ptr[4] * 10))
}
#endif // _OPENACC
