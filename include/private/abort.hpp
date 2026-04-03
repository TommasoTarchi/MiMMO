/**
 * @file abort.hpp
 *
 * @brief Declaration of error handling utilities.
 *
 * Internal utilities for aborting the program with formatted error messages.
 * Used when invalid operations are detected (e.g., null pointers, tracking
 * errors).
 *
 * @see abort.cpp for implementation
 */

#pragma once

#include <string>

namespace MiMMO {

/**
 * @brief Aborts the program displaying an error message.
 *
 * @param message Error message to be displayed.
 */
void abort_mimmo(const std::string message);

} // namespace MiMMO
