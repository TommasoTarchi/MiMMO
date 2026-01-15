/**
 * @file unit_tests_main.cpp
 *
 * @brief Unit tests for the DualMemoryManager library, using Catch2.
 */

#include "../include/dual_memory_manager/dual_memory_manager.hpp"
#include <catch2/catch_test_macros.hpp>

/**
 * @brief Simple struct for library testing.
 */
struct test_struct {
  double first_field;
  int second_field;
};

/**
 * @brief Test using basic types ('int' and 'float') without GPU
 * support.
 */
TEST_CASE("Base types - No device", "[dual_memory_manager]") {
  DualMemoryManager::DualMemoryManager dual_memory_manager =
      DualMemoryManager::DualMemoryManager();

  DualMemoryManager::DualArray<int> first_test_array =
      dual_memory_manager.allocate<int>("first_test_array", 10, false);

  dual_memory_manager.report_memory_usage();

  DualMemoryManager::DualArray<float> second_test_array =
      dual_memory_manager.allocate<float>("second_test_array", 20, false);

  dual_memory_manager.report_memory_usage();

  dual_memory_manager.free<int>(first_test_array);

  dual_memory_manager.report_memory_usage();

  dual_memory_manager.free<float>(second_test_array);

  dual_memory_manager.report_memory_usage();

  REQUIRE(true);
}

/**
 * @brief Test using test struct without GPU support.
 */
TEST_CASE("Struct - No device", "[dual_memory_manager]") {
  DualMemoryManager::DualMemoryManager dual_memory_manager =
      DualMemoryManager::DualMemoryManager();

  DualMemoryManager::DualArray<test_struct> test_array =
      dual_memory_manager.allocate<test_struct>("test_array", 10, false);

  dual_memory_manager.report_memory_usage();

  dual_memory_manager.free<test_struct>(test_array);

  dual_memory_manager.report_memory_usage();

  REQUIRE(true);
}
