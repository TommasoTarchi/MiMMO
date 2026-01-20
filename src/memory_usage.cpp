/**
 * @file memory_usage.cpp
 *
 * @brief Implementation of functions for reporting memory usage
 * (memory tracker, total memory, ...).
 */

#include "../include/mimmo/api.hpp"
#include <iomanip>
#include <iostream>

namespace MiMMO {

/**
 * @brief Returns the total host and device memory allocated by
 * the memory manager.
 *
 * @details
 * This function returns a pair where the elements are the total
 * memory allocated on host and device, respectively.
 *
 * @return (total host memory used, total device memory used)
 */
std::pair<size_t, size_t> DualMemoryManager::return_total_memory_usage() {
  /* build return pair (used host and device memory) */
  const std::pair<size_t, size_t> memory_usage(total_host_memory,
                                               total_device_memory);

  return memory_usage;
}

/**
 * @brief Reports memory used by the memory manager.
 *
 * @details
 * This function prints to standard output a complete report of memory
 * usage of the memory manager.
 *
 * A list of all allocated arrays is shown, with size (in bytes) and
 * whether the array is present on device or not.
 */
void DualMemoryManager::report_memory_usage() {

  /* define header */
  const std::string label_header = "Array Label";
  const std::string size_header = "Size (bytes)";
  const std::string on_device_header = "On Device";

  /* set width of columns */
  size_t label_col_width = label_header.length();

  for (const auto &[label, element] : memory_tracker)
    label_col_width = std::max(label_col_width, label.length());

  label_col_width += 4;

  const size_t size_col_width =
      std::max(size_header.length() + 4, static_cast<size_t>(10));
  const size_t on_device_col_width =
      std::max(on_device_header.length(), static_cast<size_t>(10));

  /* define graphic separators */
  const std::string big_separator =
      std::string(label_col_width + size_col_width + on_device_col_width, '=') +
      "\n";
  const std::string small_separator =
      std::string(label_col_width + size_col_width + on_device_col_width, '-') +
      "\n";

  /* print header */
  std::cout << "\n" << big_separator;
  std::cout << "DualMemoryManager Report:\n";
  std::cout << big_separator;
  std::cout << std::left << std::setw(label_col_width) << label_header
            << std::setw(size_col_width) << size_header
            << std::setw(on_device_col_width) << on_device_header << "\n";
  std::cout << small_separator;

  /* print tracker's content */
  for (const auto &[label, element] : memory_tracker) {
    const std::string on_device = element.second ? "yes" : "no";
    std::cout << std::left << std::setw(label_col_width) << label
              << std::setw(size_col_width) << element.first
              << std::setw(on_device_col_width) << on_device << "\n";
  }
  std::cout << big_separator;

  /* print total memory usage */
  std::cout << "Total host memory used: " << total_host_memory << " bytes"
            << "\n";
  std::cout << "Total device memory used: " << total_device_memory << " bytes"
            << "\n";
  std::cout << big_separator << "\n";

  return;
}

} // namespace MiMMO
