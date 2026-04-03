/**
 * @file allvars.hpp
 *
 * @brief Declarations of global variables for the globals example.
 *
 * Defines global extern variables that will be managed by MiMMO.
 *
 * @ingroup examples
 *
 * @see examples/globals/main.cpp
 */

#pragma once

#include "mimmo/api.hpp"

extern int global_array_size;
extern MiMMO::DualArray<int> global_array;
extern MiMMO::DualScalar<int> global_scalar;
