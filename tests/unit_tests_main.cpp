/**
 * @file unit_tests_main.cpp
 *
 * @brief Unit tests for the MiMMO library using Catch2.
 *
 * Tests cover:
 * - Basic memory allocation and deallocation
 * - Dual array metadata (size and size in bytes)
 * - Host-only allocations (dual objects not present on device)
 * - Memory copy operations (host-to-device and device-to-host)
 * - Partial memory copies, with and without offset
 * - Scalar creation and updates
 * - Memory usage reports and total memory usage
 * - Independence of distinct memory managers
 * - Memory tracker behavior on duplicate and untracked entries
 * - Macro functionality (MIMMO_GET_PTR, MIMMO_GET_VALUE, MIMMO_PRESENT)
 * - Dual objects passed to a subroutine called on device
 *
 * @see DualMemoryManager
 * @see DualArray
 * @see DualScalar
 */

#include "../include/mimmo/api.hpp"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>

/**
 * @brief Simple struct for library testing.
 */
struct test_struct {
  double first_field;
  int second_field;
};

/**
 * @brief Returns everything the given callable prints to standard output.
 *
 * @tparam F       Type of the callable to be run.
 *
 * @param function Callable printing to standard output.
 */
template <typename F> std::string capture_stdout(F function) {
  std::ostringstream buffer;
  std::streambuf *const old_buffer = std::cout.rdbuf(buffer.rdbuf());

  function();

  std::cout.rdbuf(old_buffer);

  return buffer.str();
}

/**
 * @brief Returns the number of occurrences of a substring in a string.
 *
 * @param text      String to be searched.
 * @param substring Substring to be counted.
 */
size_t count_occurrences(const std::string text, const std::string substring) {
  size_t count = 0;
  size_t position = text.find(substring);

  while (position != std::string::npos) {
    count++;
    position = text.find(substring, position + substring.length());
  }

  return count;
}

/**
 * @brief Returns the report line containing the given substring.
 *
 * @param report    Memory usage report.
 * @param substring Substring identifying the line.
 */
std::string extract_line(const std::string report,
                         const std::string substring) {
  const size_t start = report.find(substring);

  if (start == std::string::npos)
    return "";

  return report.substr(start, report.find('\n', start) - start);
}

/* subroutine scaling a dual array by a dual scalar (done sequentially
 * for simplicity)
 * */
#pragma acc routine
template <typename T>
void scale_array(const MiMMO::DualArray<T> dual_array,
                 const MiMMO::DualScalar<T> dual_scalar) {

#pragma acc loop
  for (size_t i = 0; i < dual_array.size; i++)
    MIMMO_GET_PTR(dual_array)[i] *= MIMMO_GET_VALUE(dual_scalar);

  return;
}

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

/**
 * @brief Dual array metadata test.
 */
TEST_CASE("Dual array metadata", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<double> double_array;
  memory_manager.alloc_array(double_array, "double_array", 7, true);
  MiMMO::DualArray<char> char_array;
  memory_manager.alloc_array(char_array, "char_array", 3, true);
  MiMMO::DualArray<test_struct> struct_array;
  memory_manager.alloc_array(struct_array, "struct_array", 4, true);

  REQUIRE((double_array.size == 7 &&
           double_array.size_bytes == 7 * sizeof(double)));
  REQUIRE((char_array.size == 3 && char_array.size_bytes == 3 * sizeof(char)));
  REQUIRE((struct_array.size == 4 &&
           struct_array.size_bytes == 4 * sizeof(test_struct)));

  /* host memory must be writable over the whole declared size */
  for (size_t i = 0; i < double_array.size; i++)
    double_array.host_ptr[i] = static_cast<double>(i);

  REQUIRE((double_array.host_ptr[0] == 0.0 && double_array.host_ptr[6] == 6.0));

  const std::pair<size_t, size_t> tot_mem_usage =
      memory_manager.return_total_memory_usage();

  REQUIRE(tot_mem_usage.first == double_array.size_bytes +
                                     char_array.size_bytes +
                                     struct_array.size_bytes);

  memory_manager.free_array(double_array);
  memory_manager.free_array(char_array);
  memory_manager.free_array(struct_array);
}

/**
 * @brief Host-only allocation test (default value of 'on_device').
 */
TEST_CASE("Host-only allocations", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<int> test_array;
  memory_manager.alloc_array(test_array, "test_array", 6);
  MiMMO::DualScalar<double> test_scalar;
  memory_manager.create_scalar(test_scalar, "test_scalar", 1.5);

  const std::pair<size_t, size_t> tot_mem_usage_1 =
      memory_manager.return_total_memory_usage();

  REQUIRE((test_array.host_ptr != nullptr && test_array.dev_ptr == nullptr));
  REQUIRE((test_scalar.host_value == 1.5 && test_scalar.dev_ptr == nullptr));
  REQUIRE((tot_mem_usage_1.first == 6 * sizeof(int) + sizeof(double) &&
           tot_mem_usage_1.second == 0));

  memory_manager.free_array(test_array);
  memory_manager.destroy_scalar(test_scalar);

  const std::pair<size_t, size_t> tot_mem_usage_2 =
      memory_manager.return_total_memory_usage();

  REQUIRE((test_array.host_ptr == nullptr && tot_mem_usage_2.first == 0 &&
           tot_mem_usage_2.second == 0));
}

/**
 * @brief Memory movements test with non-zero offset (host-to-device).
 */
TEST_CASE("Memcopy - offset host to device", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<int> test_array;
  memory_manager.alloc_array(test_array, "test_array", 8, true);

  for (int i = 0; i < 8; i++)
    test_array.host_ptr[i] = i;

  memory_manager.update_array_host_to_device(test_array, 0, test_array.size);

  /* change two elements on host and copy back only those */
  test_array.host_ptr[3] = 100;
  test_array.host_ptr[4] = 200;

  memory_manager.update_array_host_to_device(test_array, 3, 2);

  /* overwrite the host buffer, then restore it from device */
  for (int i = 0; i < 8; i++)
    test_array.host_ptr[i] = -1;

  memory_manager.update_array_device_to_host(test_array, 0, test_array.size);

#ifdef _OPENACC
  REQUIRE((test_array.host_ptr[0] == 0 && test_array.host_ptr[1] == 1 &&
           test_array.host_ptr[2] == 2 && test_array.host_ptr[3] == 100 &&
           test_array.host_ptr[4] == 200 && test_array.host_ptr[5] == 5 &&
           test_array.host_ptr[6] == 6 && test_array.host_ptr[7] == 7));
#else
  /* without OpenACC copies are no-ops, so the host buffer is untouched */
  REQUIRE((test_array.host_ptr[0] == -1 && test_array.host_ptr[3] == -1 &&
           test_array.host_ptr[4] == -1 && test_array.host_ptr[7] == -1));
#endif // _OPENACC

  memory_manager.free_array(test_array);
}

/**
 * @brief Memory movements test with non-zero offset (device-to-host).
 */
TEST_CASE("Memcopy - offset device to host", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<int> test_array;
  memory_manager.alloc_array(test_array, "test_array", 8, true);

  for (int i = 0; i < 8; i++)
    test_array.host_ptr[i] = i;

  memory_manager.update_array_host_to_device(test_array, 0, test_array.size);

  /* overwrite the host buffer, then restore only three elements */
  for (int i = 0; i < 8; i++)
    test_array.host_ptr[i] = -1;

  memory_manager.update_array_device_to_host(test_array, 2, 3);

#ifdef _OPENACC
  REQUIRE((test_array.host_ptr[0] == -1 && test_array.host_ptr[1] == -1 &&
           test_array.host_ptr[2] == 2 && test_array.host_ptr[3] == 3 &&
           test_array.host_ptr[4] == 4 && test_array.host_ptr[5] == -1 &&
           test_array.host_ptr[6] == -1 && test_array.host_ptr[7] == -1));
#else
  /* without OpenACC copies are no-ops, so the host buffer is untouched */
  REQUIRE((test_array.host_ptr[2] == -1 && test_array.host_ptr[3] == -1 &&
           test_array.host_ptr[4] == -1));
#endif // _OPENACC

  memory_manager.free_array(test_array);
}

/**
 * @brief Memory movements test with no elements to be copied.
 */
TEST_CASE("Memcopy - zero elements", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<int> test_array;
  memory_manager.alloc_array(test_array, "test_array", 4, true);

  for (int i = 0; i < 4; i++)
    test_array.host_ptr[i] = i;

  memory_manager.update_array_host_to_device(test_array, 0, test_array.size);

  for (int i = 0; i < 4; i++)
    test_array.host_ptr[i] = -1;

  /* empty copies must leave both host and device buffers untouched */
  memory_manager.update_array_host_to_device(test_array, 0, 0);
  memory_manager.update_array_device_to_host(test_array, 0, 0);

  REQUIRE((test_array.host_ptr[0] == -1 && test_array.host_ptr[1] == -1 &&
           test_array.host_ptr[2] == -1 && test_array.host_ptr[3] == -1));

  memory_manager.update_array_device_to_host(test_array, 0, test_array.size);

#ifdef _OPENACC
  REQUIRE((test_array.host_ptr[0] == 0 && test_array.host_ptr[1] == 1 &&
           test_array.host_ptr[2] == 2 && test_array.host_ptr[3] == 3));
#else
  REQUIRE((test_array.host_ptr[0] == -1 && test_array.host_ptr[3] == -1));
#endif // _OPENACC

  memory_manager.free_array(test_array);
}

/**
 * @brief Scalar update from device to host test.
 */
TEST_CASE("Scalar value update - device to host", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualScalar<int> test_scalar;
  memory_manager.create_scalar(test_scalar, "test_scalar", 7, true);

  /* change the value on device only */
#ifdef _OPENACC
  const int new_value = 42;
  acc_memcpy_to_device(test_scalar.dev_ptr, &new_value, sizeof(int));
#endif // _OPENACC

  const int test_value_1 = test_scalar.host_value;

  memory_manager.update_scalar_device_to_host(test_scalar);

  const int test_value_2 = test_scalar.host_value;

#ifdef _OPENACC
  REQUIRE((test_value_1 == 7 && test_value_2 == 42));
#else
  REQUIRE((test_value_1 == 7 && test_value_2 == 7));
#endif // _OPENACC

  memory_manager.destroy_scalar(test_scalar);
}

/**
 * @brief Memory usage report content test.
 */
TEST_CASE("Memory report - content", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<int> test_array;
  memory_manager.alloc_array(test_array, "reported_array", 4, true);
  MiMMO::DualScalar<int> test_scalar;
  memory_manager.create_scalar(test_scalar, "reported_scalar", 5, false);

  const std::string report =
      capture_stdout([&]() { memory_manager.report_memory_usage(); });

  REQUIRE(report.find("DualMemoryManager Report:") != std::string::npos);

  const std::string array_line = extract_line(report, "reported_array");
  const std::string scalar_line = extract_line(report, "reported_scalar");

  REQUIRE(array_line.find(std::to_string(4 * sizeof(int))) !=
          std::string::npos);
  REQUIRE(scalar_line.find(std::to_string(sizeof(int))) != std::string::npos);

  /* the scalar is host-only, so its entry is always reported as such */
  REQUIRE(scalar_line.find("no") != std::string::npos);

  REQUIRE(report.find("Total host memory used: " +
                      std::to_string(4 * sizeof(int) + sizeof(int)) +
                      " bytes") != std::string::npos);

#ifdef _OPENACC
  REQUIRE(array_line.find("yes") != std::string::npos);
  REQUIRE(report.find(
              "Total device memory used: " + std::to_string(4 * sizeof(int)) +
              " bytes") != std::string::npos);
#else
  REQUIRE(array_line.find("no") != std::string::npos);
  REQUIRE(report.find("Total device memory used: 0 bytes") !=
          std::string::npos);
#endif // _OPENACC

  memory_manager.free_array(test_array);
  memory_manager.destroy_scalar(test_scalar);
}

/**
 * @brief Memory usage report test on an empty memory manager.
 */
TEST_CASE("Memory report - empty manager", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  const std::pair<size_t, size_t> tot_mem_usage =
      memory_manager.return_total_memory_usage();

  const std::string report =
      capture_stdout([&]() { memory_manager.report_memory_usage(); });

  REQUIRE((tot_mem_usage.first == 0 && tot_mem_usage.second == 0));
  REQUIRE(report.find("Total host memory used: 0 bytes") != std::string::npos);
  REQUIRE(report.find("Total device memory used: 0 bytes") !=
          std::string::npos);
}

/**
 * @brief Memory usage report test with a label longer than the column header.
 */
TEST_CASE("Memory report - long label", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  const std::string long_label = "a_very_long_label_for_the_report_columns";

  MiMMO::DualArray<int> test_array;
  memory_manager.alloc_array(test_array, long_label, 4, true);

  const std::string report =
      capture_stdout([&]() { memory_manager.report_memory_usage(); });

  /* the label must be reported in full */
  const std::string header_line = extract_line(report, "Label");
  const std::string array_line = extract_line(report, long_label);

  REQUIRE(array_line.find(long_label) != std::string::npos);

  /* the label column must be widened to keep the size column aligned with
   * the header
   * */
  const size_t header_size_position = header_line.find("Size (bytes)");
  const size_t array_size_position =
      array_line.find(std::to_string(test_array.size_bytes));

  REQUIRE(header_size_position != std::string::npos);
  REQUIRE(array_size_position == header_size_position);

  memory_manager.free_array(test_array);
}

/**
 * @brief Memory tracking test with the same label used twice.
 */
TEST_CASE("Duplicate labels", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  const std::string label = "shared_label";

  MiMMO::DualArray<int> first_test_array;
  memory_manager.alloc_array(first_test_array, label, 4, true);
  MiMMO::DualArray<int> second_test_array;
  memory_manager.alloc_array(second_test_array, label, 6, true);

  const std::pair<size_t, size_t> tot_mem_usage =
      memory_manager.return_total_memory_usage();

  const std::string report =
      capture_stdout([&]() { memory_manager.report_memory_usage(); });

  /* entries are tracked per object, not per label, so both arrays are
   * tracked and reported
   * */
  REQUIRE(tot_mem_usage.first == 10 * sizeof(int));
  REQUIRE(count_occurrences(report, label) == 2);

  memory_manager.free_array(first_test_array);
  memory_manager.free_array(second_test_array);

  REQUIRE(memory_manager.return_total_memory_usage().first == 0);
}

/**
 * @brief Memory manager independence test.
 */
TEST_CASE("Independent memory managers", "[mimmo]") {
  MiMMO::DualMemoryManager first_memory_manager = MiMMO::DualMemoryManager();
  MiMMO::DualMemoryManager second_memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<int> first_test_array;
  first_memory_manager.alloc_array(first_test_array, "first_test_array", 10,
                                   true);
  MiMMO::DualArray<int> second_test_array;
  second_memory_manager.alloc_array(second_test_array, "second_test_array", 20,
                                    false);

  const std::pair<size_t, size_t> first_tot_mem_usage =
      first_memory_manager.return_total_memory_usage();
  const std::pair<size_t, size_t> second_tot_mem_usage =
      second_memory_manager.return_total_memory_usage();

  const std::string first_report =
      capture_stdout([&]() { first_memory_manager.report_memory_usage(); });

  REQUIRE(first_tot_mem_usage.first == 10 * sizeof(int));
  REQUIRE(second_tot_mem_usage.first == 20 * sizeof(int));
  REQUIRE(second_tot_mem_usage.second == 0);
  REQUIRE(first_report.find("second_test_array") == std::string::npos);

  first_memory_manager.free_array(first_test_array);

  /* freeing from one manager must not affect the other */
  REQUIRE(first_memory_manager.return_total_memory_usage().first == 0);
  REQUIRE(second_memory_manager.return_total_memory_usage().first ==
          20 * sizeof(int));

  second_memory_manager.free_array(second_test_array);
}

/**
 * @brief Reallocation test of a dual array that was previously freed.
 */
TEST_CASE("Reallocation after free", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<int> test_array;
  memory_manager.alloc_array(test_array, "test_array", 4, true);
  memory_manager.free_array(test_array);

  REQUIRE(test_array.host_ptr == nullptr);

  /* the same object can be tracked again once it has been freed */
  memory_manager.alloc_array(test_array, "test_array_again", 9, true);

  for (int i = 0; i < 9; i++)
    test_array.host_ptr[i] = i;

  const std::pair<size_t, size_t> tot_mem_usage =
      memory_manager.return_total_memory_usage();

  REQUIRE((test_array.size == 9 && test_array.size_bytes == 9 * sizeof(int) &&
           test_array.host_ptr[8] == 8));
  REQUIRE(tot_mem_usage.first == 9 * sizeof(int));

  memory_manager.free_array(test_array);

  REQUIRE(memory_manager.return_total_memory_usage().first == 0);
}

/**
 * @brief Memory tracker test on insertion of an already tracked object.
 */
TEST_CASE("Memory tracker - duplicate insertion", "[mimmo]") {
  std::map<void *, std::tuple<std::string, size_t, bool>> memory_tracker;
  std::pair<size_t, size_t> tot_memory_usage = {0, 0};

  int test_object = 0;

  REQUIRE_FALSE(MiMMO::add_to_memory_tracker(memory_tracker, tot_memory_usage,
                                             (void *)&test_object,
                                             "test_object", 16, true));
  REQUIRE((tot_memory_usage.first == 16 && tot_memory_usage.second == 16));

  /* a second insertion of the same object must fail and leave both the
   * tracker and the totals untouched
   * */
  REQUIRE(MiMMO::add_to_memory_tracker(memory_tracker, tot_memory_usage,
                                       (void *)&test_object,
                                       "test_object_again", 32, true));
  REQUIRE(memory_tracker.size() == 1);
  REQUIRE((tot_memory_usage.first == 16 && tot_memory_usage.second == 16));
  REQUIRE(std::get<0>(memory_tracker[(void *)&test_object]) == "test_object");
}

/**
 * @brief Memory tracker test on removal of an untracked object.
 */
TEST_CASE("Memory tracker - removal", "[mimmo]") {
  std::map<void *, std::tuple<std::string, size_t, bool>> memory_tracker;
  std::pair<size_t, size_t> tot_memory_usage = {0, 0};

  int test_object = 0;
  int untracked_object = 0;

  REQUIRE_FALSE(MiMMO::add_to_memory_tracker(memory_tracker, tot_memory_usage,
                                             (void *)&test_object,
                                             "test_object", 8, false));

  /* host-only entries must not contribute to device memory usage */
  REQUIRE((tot_memory_usage.first == 8 && tot_memory_usage.second == 0));

  /* removal of an untracked object must fail and leave the totals
   * untouched
   * */
  REQUIRE(MiMMO::remove_from_memory_tracker(memory_tracker, tot_memory_usage,
                                            (void *)&untracked_object));
  REQUIRE((memory_tracker.size() == 1 && tot_memory_usage.first == 8));

  REQUIRE_FALSE(MiMMO::remove_from_memory_tracker(
      memory_tracker, tot_memory_usage, (void *)&test_object));
  REQUIRE(memory_tracker.empty());
  REQUIRE((tot_memory_usage.first == 0 && tot_memory_usage.second == 0));

  /* a second removal of the same object must fail */
  REQUIRE(MiMMO::remove_from_memory_tracker(memory_tracker, tot_memory_usage,
                                            (void *)&test_object));
  REQUIRE((tot_memory_usage.first == 0 && tot_memory_usage.second == 0));
}

/**
 * @brief Test of dual objects passed to a subroutine called on device.
 */
TEST_CASE("Subroutine call", "[mimmo]") {
  MiMMO::DualMemoryManager memory_manager = MiMMO::DualMemoryManager();

  MiMMO::DualArray<int> test_array;
  memory_manager.alloc_array(test_array, "test_array", 5, true);
  MiMMO::DualScalar<int> test_scalar;
  memory_manager.create_scalar(test_scalar, "test_scalar", 3, true);

  for (int i = 0; i < 5; i++)
    test_array.host_ptr[i] = i;

  memory_manager.update_array_host_to_device(test_array, 0, test_array.size);

#pragma acc parallel MIMMO_PRESENT(test_array)                                 \
    MIMMO_PRESENT(test_scalar) default(none)
  {
    scale_array(test_array, test_scalar);
  }

  memory_manager.update_array_device_to_host(test_array, 0, test_array.size);

  /* without OpenACC the subroutine operates on host memory, so the result
   * is the same in both cases
   * */
  REQUIRE((test_array.host_ptr[0] == 0 && test_array.host_ptr[1] == 3 &&
           test_array.host_ptr[2] == 6 && test_array.host_ptr[3] == 9 &&
           test_array.host_ptr[4] == 12));

  memory_manager.free_array(test_array);
  memory_manager.destroy_scalar(test_scalar);
}
