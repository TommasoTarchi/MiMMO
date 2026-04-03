/**
 * @file unit_tests_main.cpp
 *
 * @brief Unit tests for the MiMMO library using Catch2.
 *
 * Tests cover:
 * - Basic memory allocation and deallocation
 * - Memory copy operations (host-to-device and device-to-host)
 * - Partial memory copies
 * - Scalar creation and updates
 * - Macro functionality (MIMMO_GET_PTR, MIMMO_GET_VALUE, MIMMO_PRESENT)
 *
 * @see DualMemoryManager
 * @see DualArray
 * @see DualScalar
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
 * @brief Memory manager test using basic types.
 */
TEST_CASE("Memory manager - base types", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  const size_t first_size = 10 * sizeof(int);
  const size_t second_size = 20 * sizeof(float);

  MiMMO::DualArray<int> first_test_array;
  memory_manager.alloc_array(first_test_array, "first_test_array", 10, true);
  MiMMO::DualScalar<int> first_test_scalar;
  memory_manager.create_scalar(first_test_scalar, "first_test_scalar", 3, true);

  memory_manager.report_memory_usage();
  const std::pair<size_t, size_t> tot_mem_usage_1 =
      memory_manager.return_total_memory_usage();

  MiMMO::DualArray<float> second_test_array;
  memory_manager.alloc_array(second_test_array, "second_test_array", 20, false);
  MiMMO::DualScalar<float> second_test_scalar;
  memory_manager.create_scalar(second_test_scalar, "second_test_scalar", 3.14f,
                               false);

  memory_manager.report_memory_usage();
  const std::pair<size_t, size_t> tot_mem_usage_2 =
      memory_manager.return_total_memory_usage();

  memory_manager.free_array(first_test_array);

  memory_manager.report_memory_usage();
  const std::pair<size_t, size_t> tot_mem_usage_3 =
      memory_manager.return_total_memory_usage();

  memory_manager.free_array(second_test_array);
  memory_manager.destroy_scalar(first_test_scalar);
  memory_manager.destroy_scalar(second_test_scalar);

  memory_manager.report_memory_usage();
  const std::pair<size_t, size_t> tot_mem_usage_4 =
      memory_manager.return_total_memory_usage();

#ifdef _OPENACC
  REQUIRE((tot_mem_usage_1.first == first_size + sizeof(int) &&
           tot_mem_usage_1.second == first_size + sizeof(int) &&
           tot_mem_usage_2.first ==
               first_size + second_size + sizeof(int) + sizeof(float) &&
           tot_mem_usage_2.second == first_size + sizeof(int) &&
           tot_mem_usage_3.first == second_size + sizeof(int) + sizeof(float) &&
           tot_mem_usage_3.second == sizeof(int) &&
           tot_mem_usage_4.first == 0 && tot_mem_usage_4.second == 0));
#else
  REQUIRE((tot_mem_usage_1.first == first_size + sizeof(int) &&
           tot_mem_usage_1.second == 0 &&
           tot_mem_usage_2.first ==
               first_size + second_size + sizeof(int) + sizeof(float) &&
           tot_mem_usage_2.second == 0 &&
           tot_mem_usage_3.first == second_size + sizeof(int) + sizeof(float) &&
           tot_mem_usage_3.second == 0 && tot_mem_usage_4.first == 0 &&
           tot_mem_usage_4.second == 0));
#endif // _OPENACC
}

/**
 * @brief Memory manager test using test struct.
 */
TEST_CASE("Memory manager - struct", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  const size_t size = 10 * sizeof(test_struct);

  MiMMO::DualArray<test_struct> test_array;
  memory_manager.alloc_array(test_array, "test_array", 10, true);
  MiMMO::DualScalar<test_struct> test_scalar;
  memory_manager.create_scalar(test_scalar, "test_scalar", {1.0, 2}, true);

  memory_manager.report_memory_usage();
  const std::pair<size_t, size_t> tot_mem_usage_1 =
      memory_manager.return_total_memory_usage();

  memory_manager.free_array(test_array);

  memory_manager.report_memory_usage();
  const std::pair<size_t, size_t> tot_mem_usage_2 =
      memory_manager.return_total_memory_usage();

  memory_manager.destroy_scalar(test_scalar);

  memory_manager.report_memory_usage();
  const std::pair<size_t, size_t> tot_mem_usage_3 =
      memory_manager.return_total_memory_usage();

#ifdef _OPENACC
  REQUIRE((tot_mem_usage_1.first == size + sizeof(test_struct) &&
           tot_mem_usage_1.second == size + sizeof(test_struct) &&
           tot_mem_usage_2.first == sizeof(test_struct) &&
           tot_mem_usage_2.second == sizeof(test_struct) &&
           tot_mem_usage_3.first == 0 && tot_mem_usage_3.second == 0));
#else
  REQUIRE((tot_mem_usage_1.first == size + sizeof(test_struct) &&
           tot_mem_usage_1.second == 0 &&
           tot_mem_usage_2.first == sizeof(test_struct) &&
           tot_mem_usage_2.second == 0 && tot_mem_usage_3.first == 0 &&
           tot_mem_usage_3.second == 0));
#endif // _OPENACC
}

/**
 * @brief Memory movements test (host-to-device and device-to-host).
 */
TEST_CASE("Memcopy", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<int> test_array;
  memory_manager.alloc_array(test_array, "test_array", 5, true);
  MiMMO::DualArray<int> test_array_copy;
  memory_manager.alloc_array(test_array_copy, "test_array_copy", 5, true);

  for (int i = 0; i < 5; i++) {
    test_array.host_ptr[i] = i;
    test_array_copy.host_ptr[i] = i;
  }

  memory_manager.update_array_host_to_device(test_array, 0, test_array.size);

#pragma acc parallel copyin(test_array)                                        \
    deviceptr(test_array.dev_ptr) default(none)
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

  MiMMO::DualArray<int> test_array;
  memory_manager.alloc_array(test_array, "test_array", 5, true);
  MiMMO::DualArray<int> test_array_copy;
  memory_manager.alloc_array(test_array_copy, "test_array_copy", 5, true);

  for (int i = 0; i < 5; i++) {
    test_array.host_ptr[i] = i;
    test_array_copy.host_ptr[i] = i;
  }

  memory_manager.update_array_host_to_device(test_array, 0, test_array.size);

#pragma acc parallel copyin(test_array)                                        \
    deviceptr(test_array.dev_ptr) default(none)
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

  MiMMO::DualScalar<int> test_scalar;
  memory_manager.create_scalar(test_scalar, "test_scalar", 100, true);

  const int test_value_host_1 = test_scalar.host_value;
#ifdef _OPENACC
  int test_value_dev_1 = 0;
  acc_memcpy_from_device(&test_value_dev_1, test_scalar.dev_ptr, sizeof(int));
#endif // _OPENACC

  test_scalar.host_value = 200;

  const int test_value_host_2 = test_scalar.host_value;
#ifdef _OPENACC
  int test_value_dev_2 = 0;
  acc_memcpy_from_device(&test_value_dev_2, test_scalar.dev_ptr, sizeof(int));
#endif // _OPENACC

  memory_manager.update_scalar_host_to_device(test_scalar);

  const int test_value_host_3 = test_scalar.host_value;
#ifdef _OPENACC
  int test_value_dev_3 = 0;
  acc_memcpy_from_device(&test_value_dev_3, test_scalar.dev_ptr, sizeof(int));
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

  MiMMO::DualArray<int> test_array;
  memory_manager.alloc_array(test_array, "test_array", 10, true);

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

  MiMMO::DualScalar<int> test_scalar;
  memory_manager.create_scalar(test_scalar, "test_scalar", 100, true);

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

/**
 * @brief Present macro test for OpenACC pragmas.
 */
TEST_CASE("Present macro", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<int> test_array;
  memory_manager.alloc_array(test_array, "test_array", 5, true);
  MiMMO::DualScalar<int> test_scalar;
  memory_manager.create_scalar(test_scalar, "test_scalar", 10, true);

  for (int i = 0; i < 5; i++)
    test_array.host_ptr[i] = i;

  memory_manager.update_array_host_to_device(test_array, 0, test_array.size);

  for (int i = 0; i < 5; i++)
    test_array.host_ptr[i] += 1;

#pragma acc parallel MIMMO_PRESENT(test_array)                                 \
    MIMMO_PRESENT(test_scalar) default(none)
  {
#pragma acc loop
    for (int i = 0; i < test_array.size; i++)
      MIMMO_GET_PTR(test_array)[i] *= MIMMO_GET_VALUE(test_scalar);

    MIMMO_GET_VALUE(test_scalar) += 5;
  }

  test_scalar.host_value += 5;

  memory_manager.update_array_device_to_host(test_array, 0, test_array.size);
  memory_manager.update_scalar_device_to_host(test_scalar);

#ifdef _OPENACC
  REQUIRE((test_array.host_ptr[0] == 0 && test_array.host_ptr[1] == 10 &&
           test_array.host_ptr[2] == 20 && test_array.host_ptr[3] == 30 &&
           test_array.host_ptr[4] == 40 && test_scalar.host_value == 15));
#else
  REQUIRE((test_array.host_ptr[0] == 10 && test_array.host_ptr[1] == 20 &&
           test_array.host_ptr[2] == 30 && test_array.host_ptr[3] == 40 &&
           test_array.host_ptr[4] == 50 && test_scalar.host_value == 20));
#endif // _OPENACC

  memory_manager.free_array(test_array);
  memory_manager.destroy_scalar(test_scalar);
}
