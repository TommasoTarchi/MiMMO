// TODO: add file description

#include "../include/api/dual_memory_manager.hpp"
#include <iomanip>
#include <iostream>

namespace DualMemoryManager {

// TODO: add description
void DualMemoryManager::report_memory_usage(void) {

  /* define header */
  const std::string label_header = "Array Label";
  const std::string size_header = "Size (bytes)";
  const std::string on_device_header = "On Device";

  /* set width of columns */
  size_t label_col_width = label_header.length();

  for (const auto &[label, size] : host_memory_tracker)
    label_col_width = std::max(label_col_width, label.length());

  label_col_width += 4;

  size_t size_col_width = 10;
  size_t on_device_col_width = 10;

  /* define graphic separators */
  const std::string big_separator =
      std::string(label_col_width + size_col_width + on_device_col_width, '=') +
      "\n";
  const std::string small_separator =
      std::string(label_col_width + size_col_width + on_device_col_width, '-') +
      "\n";

  /* print header */
  std::cout << big_separator;
  std::cout << "DualMemoryManager Report:\n";
  std::cout << small_separator;
  std::cout << std::left << std::setw(label_col_width) << label_header
            << std::setw(size_col_width) << size_header
            << std::setw(on_device_col_width) << on_device_header << "\n";
  std::cout << small_separator;

  /* print table content */
  for (const auto &[label, size] : host_memory_tracker) {
    /* check if the array is present on device as well */
    const auto it = device_memory_tracker.find(label);
    const std::string on_device =
        it == device_memory_tracker.end() ? "no" : "yes";

    /* print data */
    std::cout << std::left << std::setw(label_col_width) << label
              << std::setw(size_col_width) << size
              << std::setw(on_device_col_width) << on_device << "\n";
    std::cout << big_separator;
  }

  return;
}

} // namespace DualMemoryManager
