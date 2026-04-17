# MiMMO Agent Instructions

## Build and Test Commands

### Prerequisites
- CMake ≥ 3.15
- NVIDIA HPC SDK (NVHPC) 25.x recommended (`CXX=nvc++`)
- For CPU-only builds: any C++17 compiler

### Build Commands
```bash
# Configure and build
mkdir build && cmake -S . -B build && cmake --build build

# Build with GPU support disabled
cmake -S . -B build -DOPENACC=OFF

# Build without tests
cmake -S . -B build -DUNIT_TESTS=OFF

# Generate documentation
cmake --build build --target docs
```

### Test Commands
```bash
# Run all tests
ctest --test-dir build --output-on-failure

# Run a specific test by name (Catch2)
ctest --test-dir build -R test_name --output-on-failure

# Run with verbose output
ctest --test-dir build -V --output-on-failure
```

The test executable is `build/unit_tests.x`. Individual tests are discovered by Catch2.

## Code Style Guidelines

### General Style
- **Formatting**: Follow `.clang-format` configuration (2-space indentation, line width 80)
- **Pre-commit**: Run `clang-format` automatically via pre-commit hooks
- **Line length**: Maximum 80 characters for code

### C++ Standards
- **C++17** required with standard features

### Naming Conventions
- **Classes**: `PascalCase` (e.g., `DualMemoryManager`, `DualArray`)
- **Structs**: `PascalCase` (e.g., `DualArray`, `DualScalar`)
- **Functions**: `snake_case` (e.g., `alloc_array`, `update_array_host_to_device`)
- **Variables**: `snake_case` (e.g., `host_ptr`, `memory_tracker`)
- **Constants**: `UPPER_SNAKE_CASE` where applicable
- **Namespaces**: `MiMMO` (no suffix)

### Includes and Headers
- Include guards: Use `#pragma once`
- Public headers: `include/mimmo/` directory
- Private/inl headers: `include/private/` directory
- Order: Standard library first, then project headers
- Self-contained headers with all necessary includes

### Documentation
- Use Doxygen comments for all public APIs
- Include `@brief` and `@details` tags
- Use `@tparam` for templates
- Include `@see` references to related documentation
- Example usage in main header comments

### Error Handling
- Use `MiMMO::abort()` for critical errors in `include/private/abort.hpp`
- Invalid state access triggers program abortion
- Validate OpenACC compilation before runtime

### Templating
- Template implementations in `.inl` files
- Templates are instantiated in `api.hpp` includes
- Template functions documented in headers

### Memory Management
- `DualMemoryManager` tracks all allocations
- Host pointers allocated with RAII patterns
- Device pointers via OpenACC runtime
- Never call manager methods from device code

### OpenACC Integration
- Macros `MIMMO_GET_PTR()`, `MIMMO_GET_VALUE()`, `MIMMO_PRESENT()`
- Always use `MIMMO_PRESENT()` in pragma clauses
- Manager methods require host context only

### Test Guidelines
- Tests use Catch2 framework
- Organize by test case with `[mimmo]` tags
- Test both OpenACC and non-OpenACC paths via `#ifdef _OPENACC`
- Include memory usage verification in tests

### Git Workflow
Do **not** use git.
