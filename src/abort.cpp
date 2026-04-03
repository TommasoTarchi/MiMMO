/**
 * @file abort.cpp
 *
 * @brief Implementation of error handling utilities.
 *
 * @see abort.hpp
 */

#include "../include/private/abort.hpp"
#include <iostream>

namespace MiMMO {

/**
 * @brief Aborts the program displaying an error message.
 *
 * @param message Error message to be displayed.
 *
 * @todo Consider adding more context to error messages.
 */
void abort_mimmo(const std::string message) {

  /* make sure standard output is flushed */
  std::cout.flush();

  /* return abort message */
  std::cerr << "DualMemoryManager error: " << message << std::endl;

  /* exit program */
  exit(1);

  return;
}

} // namespace MiMMO
