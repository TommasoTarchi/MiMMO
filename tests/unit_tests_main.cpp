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

// TODO: add scalars
/**
 * @brief Memory manager test using basic types.
 */
TEST_CASE("Memory manager - base types", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  const size_t first_size = 10 * sizeof(int);
  const size_t second_size = 20 * sizeof(float);

  MiMMO::DualArray<int> first_test_array =
      memory_manager.alloc_array<int>("first_test_array", 10, true);

  memory_manager.report_memory_usage();
  const std::pair<size_t, size_t> tot_mem_usage_1 =
      memory_manager.return_total_memory_usage();

  MiMMO::DualArray<float> second_test_array =
      memory_manager.alloc_array<float>("second_test_array", 20, false);

  memory_manager.report_memory_usage();
  const std::pair<size_t, size_t> tot_mem_usage_2 =
      memory_manager.return_total_memory_usage();

  memory_manager.free_array(first_test_array);

  memory_manager.report_memory_usage();
  const std::pair<size_t, size_t> tot_mem_usage_3 =
      memory_manager.return_total_memory_usage();

  memory_manager.free_array(second_test_array);

  memory_manager.report_memory_usage();
  const std::pair<size_t, size_t> tot_mem_usage_4 =
      memory_manager.return_total_memory_usage();

#ifdef _OPENACC
  REQUIRE((tot_mem_usage_1.first == first_size &&
           tot_mem_usage_1.second == first_size &&
           tot_mem_usage_2.first == (first_size + second_size) &&
           tot_mem_usage_2.second == first_size &&
           tot_mem_usage_3.first == second_size &&
           tot_mem_usage_3.second == 0 && tot_mem_usage_4.first == 0 &&
           tot_mem_usage_4.second == 0));
#else
  REQUIRE((tot_mem_usage_1.first == first_size && tot_mem_usage_1.second == 0 &&
           tot_mem_usage_2.first == (first_size + second_size) &&
           tot_mem_usage_2.second == 0 &&
           tot_mem_usage_3.first == second_size &&
           tot_mem_usage_3.second == 0 && tot_mem_usage_4.first == 0 &&
           tot_mem_usage_4.second == 0));
#endif // _OPENACC
}

// TODO: add scalars
/**
 * @brief Memory manager test using test struct.
 */
TEST_CASE("Memory manager - struct", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  const size_t size = 10 * sizeof(test_struct);

  MiMMO::DualArray<test_struct> test_array =
      memory_manager.alloc_array<test_struct>("test_array", 10, true);

  memory_manager.report_memory_usage();
  const std::pair<size_t, size_t> tot_mem_usage_1 =
      memory_manager.return_total_memory_usage();

  memory_manager.free_array(test_array);

  memory_manager.report_memory_usage();
  const std::pair<size_t, size_t> tot_mem_usage_2 =
      memory_manager.return_total_memory_usage();

#ifdef _OPENACC
  REQUIRE((tot_mem_usage_1.first == size && tot_mem_usage_1.second == size &&
           tot_mem_usage_2.first == 0 && tot_mem_usage_2.second == 0));
#else
  REQUIRE((tot_mem_usage_1.first == size && tot_mem_usage_1.second == 0 &&
           tot_mem_usage_2.first == 0 && tot_mem_usage_2.second == 0));
#endif // _OPENACC
}

/**
 * @brief Memory movements test (host-to-device and device-to-host).
 */
TEST_CASE("Memcopy", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<int> test_array =
      memory_manager.alloc_array<int>("test_array", 5, true);
  MiMMO::DualArray<int> test_array_copy =
      memory_manager.alloc_array<int>("test_array_copy", 5, true);

  for (int i = 0; i < 5; i++) {
    test_array.host_ptr[i] = i;
    test_array_copy.host_ptr[i] = i;
  }

  memory_manager.update_array_host_to_device(test_array, 0, test_array.size);

#pragma acc parallel deviceptr(test_array.dev_ptr)
  {
#pragma acc loop
    for (int i = 0; i < 5; i++)
      MIMMO_GET_PTR(test_array)[i] *= 10;
  }

  memory_manager.update_array_device_to_host(test_array, 0, test_array.size);

  REQUIRE(((test_array.host_ptr[0] == test_array_copy.host_ptr[0] * 10) &&
           (test_array.host_ptr[1] == test_array_copy.host_ptr[1] * 10) &&
           (test_array.host_ptr[2] == test_array_copy.host_ptr[2] * 10) &&
           (test_array.host_ptr[3] == test_array_copy.host_ptr[3] * 10) &&
           (test_array.host_ptr[4] == test_array_copy.host_ptr[4] * 10)));

  memory_manager.free_array(test_array);
  memory_manager.free_array(test_array_copy);
}

/**
 * @brief Partial memory movements test (host-to-device and device-to-host).
 */
TEST_CASE("Memcopy - partial copy", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<int> test_array =
      memory_manager.alloc_array<int>("test_array", 5, true);
  MiMMO::DualArray<int> test_array_copy =
      memory_manager.alloc_array<int>("test_array_copy", 5, true);

  for (int i = 0; i < 5; i++) {
    test_array.host_ptr[i] = i;
    test_array_copy.host_ptr[i] = i;
  }

  memory_manager.update_array_host_to_device(test_array, 0, test_array.size);

#pragma acc parallel deviceptr(test_array.dev_ptr)
  {
#pragma acc loop
    for (int i = 0; i < 5; i++)
      MIMMO_GET_PTR(test_array)[i] *= 10;
  }

  memory_manager.update_array_device_to_host(test_array, 0, 3);

#ifdef _OPENACC
  REQUIRE(((test_array.host_ptr[0] == test_array_copy.host_ptr[0] * 10) &&
           (test_array.host_ptr[1] == test_array_copy.host_ptr[1] * 10) &&
           (test_array.host_ptr[2] == test_array_copy.host_ptr[2] * 10) &&
           (test_array.host_ptr[3] == test_array_copy.host_ptr[3]) &&
           (test_array.host_ptr[4] == test_array_copy.host_ptr[4])));
#else
  REQUIRE(((test_array.host_ptr[0] == test_array_copy.host_ptr[0] * 10) &&
           (test_array.host_ptr[1] == test_array_copy.host_ptr[1] * 10) &&
           (test_array.host_ptr[2] == test_array_copy.host_ptr[2] * 10) &&
           (test_array.host_ptr[3] == test_array_copy.host_ptr[3] * 10) &&
           (test_array.host_ptr[4] == test_array_copy.host_ptr[4] * 10)));
#endif // _OPENACC

  memory_manager.free_array(test_array);
  memory_manager.free_array(test_array_copy);
}

/**
 * @brief Scalar value update test.
 */
TEST_CASE("Scalar value update", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualScalar<int> test_scalar =
      memory_manager.create_scalar<int>("test_scalar", 100, true);

  const int test_value_host_1 = test_scalar.host_value;
#ifdef _OPENACC
  const int test_value_dev_1 = *test_scalar.dev_ptr;
#endif // _OPENACC

  memory_manager.update_scalar_value(test_scalar, 200, false);

  const int test_value_host_2 = test_scalar.host_value;
#ifdef _OPENACC
  const int test_value_dev_2 = *test_scalar.dev_ptr;
#endif // _OPENACC

  memory_manager.update_scalar_value(test_scalar, 200, true);

  const int test_value_host_3 = test_scalar.host_value;
#ifdef _OPENACC
  const int test_value_dev_3 = *test_scalar.dev_ptr;
#endif // _OPENACC

#ifdef _OPENACC
  REQUIRE((test_value_host_1 == 100 && test_value_dev_1 == 100 &&
           test_value_host_2 == 200 && test_value_dev_2 == 100 &&
           test_value_host_3 == 200 && test_value_dev_3 == 200));
#else
  REQUIRE((test_value_host_1 == 100 && test_value_host_2 == 200 &&
           test_value_host_3 == 200));
#endif // _OPENACC

  memory_manager.destroy_scalar(test_scalar);
}

/**
 * @brief Pointer selection macro test.
 */
TEST_CASE("Pointer selection macro", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<int> test_array =
      memory_manager.alloc_array<int>("test_array", 10, true);

  const int *ref_ptr_dev = test_array.dev_ptr;
  const int *ref_ptr_host = test_array.host_ptr;
  const int *test_ptr = MIMMO_GET_PTR(test_array);

#ifdef _OPENACC
  REQUIRE((ref_ptr_dev == test_ptr && ref_ptr_host != test_ptr));
#else
  REQUIRE((ref_ptr_dev == nullptr && ref_ptr_host == test_ptr));
#endif // _OPENACC

  memory_manager.free_array(test_array);
}

/**
 * @brief Scalar value selection macro.
 */
TEST_CASE("Value selection macro", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualScalar<int> test_scalar =
      memory_manager.create_scalar<int>("test_scalar", 100, true);

  const int *ref_ptr_dev = test_scalar.dev_ptr;
  const int *ref_ptr_host = &test_scalar.host_value;
  const int *test_ptr = &(MIMMO_GET_VALUE(test_scalar));

#ifdef _OPENACC
  REQUIRE((ref_ptr_dev == test_ptr && ref_ptr_host != test_ptr));
#else
  REQUIRE((ref_ptr_dev == nullptr && ref_ptr_host == test_ptr));
#endif // _OPENACC

  memory_manager.destroy_scalar(test_scalar);
}

// TODO: add dual scalar
/**
 * @brief Present macro test for OpenACC pragmas.
 */
TEST_CASE("Present macro test", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<int> test_array =
      memory_manager.alloc_array<int>("test_array", 5, true);

  for (int i = 0; i < 5; i++)
    test_array.host_ptr[i] = i;

  memory_manager.update_array_host_to_device(test_array, 0, test_array.size);

  for (int i = 0; i < 5; i++)
    test_array.host_ptr[i] += 1;

#pragma acc parallel MIMMO_PRESENT(test_array)
  {
#pragma acc loop
    for (int i = 0; i < test_array.size; i++)
      MIMMO_GET_PTR(test_array)[i] *= 10;
  }

  memory_manager.update_array_device_to_host(test_array, 0, test_array.size);

#ifdef _OPENACC
  REQUIRE((test_array.host_ptr[0] == 0 && test_array.host_ptr[1] == 10 &&
           test_array.host_ptr[2] == 20 && test_array.host_ptr[3] == 30 &&
           test_array.host_ptr[4] == 40));
#else
  REQUIRE((test_array.host_ptr[0] == 10 && test_array.host_ptr[1] == 20 &&
           test_array.host_ptr[2] == 30 && test_array.host_ptr[3] == 40 &&
           test_array.host_ptr[4] == 50));
#endif // _OPENACC

  memory_manager.free_array(test_array);
}
