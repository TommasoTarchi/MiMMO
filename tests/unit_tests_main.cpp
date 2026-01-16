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
TEST_CASE("Memory manager - base types - No device", "[dual_memory_manager]") {
  MiMMO::DualMemoryManager dual_memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<int> first_test_array =
      dual_memory_manager.allocate<int>("first_test_array", 10, false);

  dual_memory_manager.report_memory_usage();

  MiMMO::DualArray<float> second_test_array =
      dual_memory_manager.allocate<float>("second_test_array", 20, false);

  dual_memory_manager.report_memory_usage();

  dual_memory_manager.free(first_test_array);

  dual_memory_manager.report_memory_usage();

  dual_memory_manager.free(second_test_array);

  dual_memory_manager.report_memory_usage();

  REQUIRE(true);
}

/**
 * @brief Memory manager test using test struct without GPU support.
 */
TEST_CASE("Memory manager - struct - No device", "[dual_memory_manager]") {
  MiMMO::DualMemoryManager dual_memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<test_struct> test_array =
      dual_memory_manager.allocate<test_struct>("test_array", 10, false);

  dual_memory_manager.report_memory_usage();

  dual_memory_manager.free(test_array);

  dual_memory_manager.report_memory_usage();

  REQUIRE(true);
}

/**
 * @brief Pointer selection test without GPU support.
 */
TEST_CASE("Pointer selection - No device", "[dual_memory_manager]") {
  MiMMO::DualMemoryManager dual_memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<int> test_array =
      dual_memory_manager.allocate<int>("first_test_array", 10, false);

  int *ref_ptr = test_array.host_ptr;
  int *test_ptr = MiMMO::select_ptr(test_array);

  REQUIRE(test_ptr == ref_ptr);

  dual_memory_manager.free(test_array);
}

#ifdef _OPENACC
/**
 * @brief Memory manager test using basic types ('int' and 'float') with GPU
 * support.
 */
TEST_CASE("Memory manager - base types", "[dual_memory_manager]") {
  MiMMO::DualMemoryManager dual_memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<int> first_test_array =
      dual_memory_manager.allocate<int>("first_test_array", 10, true);

  dual_memory_manager.report_memory_usage();

  MiMMO::DualArray<float> second_test_array =
      dual_memory_manager.allocate<float>("second_test_array", 20, false);

  dual_memory_manager.report_memory_usage();

  dual_memory_manager.free(first_test_array);

  dual_memory_manager.report_memory_usage();

  dual_memory_manager.free(second_test_array);

  dual_memory_manager.report_memory_usage();

  REQUIRE(true);
}

/**
 * @brief Memory manager test using test struct with GPU support.
 */
TEST_CASE("Memory manager - struct", "[dual_memory_manager]") {
  MiMMO::DualMemoryManager dual_memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<test_struct> test_array =
      dual_memory_manager.allocate<test_struct>("test_array", 10, true);

  dual_memory_manager.report_memory_usage();

  dual_memory_manager.free(test_array);

  dual_memory_manager.report_memory_usage();

  REQUIRE(true);
}

/**
 * @brief Pointer selection test with GPU support.
 */
TEST_CASE("Pointer selection", "[dual_memory_manager]") {
  MiMMO::DualMemoryManager dual_memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<int> test_array =
      dual_memory_manager.allocate<int>("first_test_array", 10, true);

  int *ref_ptr_dev = test_array.dev_ptr;
  int *ref_ptr_host = test_array.host_ptr;
  int *test_ptr = MiMMO::select_ptr(test_array);

  REQUIRE(test_ptr == ref_ptr_dev && test_ptr != ref_ptr_host);

  dual_memory_manager.free(test_array);
}
#endif // _OPENACC
