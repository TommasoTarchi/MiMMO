# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

MiMMO (Minimal Memory Manager for OpenACC) is a small C++17 library that wraps
the OpenACC runtime API to give explicit, tracked control over host/device
memory (dual arrays and dual scalars), instead of relying on OpenACC's opaque
high-level `copyin`/`copyout` data clauses.

## Git workflow

Do **not** use any `git` command in this repository (no `git status`, `git
diff`, `git add`, `git commit`, etc.). Leave version control to the user.

## Build and test commands

```bash
# Configure and build (requires CXX=nvc++ / NVHPC for OpenACC support)
mkdir build && cmake -S . -B build && cmake --build build

# Build without OpenACC (any C++17 compiler)
cmake -S . -B build -DOPENACC=OFF

# Build without the Catch2 unit test target
cmake -S . -B build -DUNIT_TESTS=OFF

# Generate Doxygen docs (output in build/docs/)
cmake --build build --target docs

# Run all tests
ctest --test-dir build --output-on-failure

# Run a single test case by name (Catch2, tag [mimmo])
ctest --test-dir build -R "test_name" --output-on-failure
```

The test executable is `build/unit_tests.x`; tests are auto-registered with
CTest via `catch_discover_tests`. Catch2 v3.4.0 is fetched automatically by
CMake (`FetchContent`) when `UNIT_TESTS=ON`.

On non-NVHPC compilers, OpenACC is unavailable, so use `-DOPENACC=OFF`
(CMake fails with `FATAL_ERROR` otherwise, since it requires
`CMAKE_CXX_COMPILER_ID MATCHES "NVHPC"`).

## Architecture

The library is intentionally tiny and split into a **public header-only API**
plus a **small compiled runtime** for tracking/reporting:

- `include/mimmo/api.hpp` — the single public entry point. Declares
  `DualArray<T>` / `DualScalar<T>` (plain structs holding host pointer/value +
  device pointer + metadata) and the `DualMemoryManager` class, plus the three
  OpenACC helper macros. It includes the `.inl` files at the bottom so that
  template method bodies are visible wherever the header is included.
- `include/private/arrays.inl` / `scalars.inl` — template method definitions
  for `DualMemoryManager` (`alloc_array`, `update_array_*`, `free_array`,
  `create_scalar`, `update_scalar_*`, `destroy_scalar`). These are templated
  on element type `T` so they must live in headers, not `.cpp` files.
  `#ifdef _OPENACC` guards branch between the real OpenACC calls
  (`acc_malloc`, `acc_memcpy_to_device`, `acc_free`, ...) and the CPU-only
  fallback (device pointers stay `nullptr`, sync calls are no-ops).
  Non-templated logic (tracking, error checks) is factored out into
  `memory_tracker.*` / `abort.*` so template instantiation stays cheap.
  Note the internal signature mismatch: `alloc_array`/`create_scalar`/
  `free_array`/`destroy_scalar` take the dual object by reference (so the
  manager can mutate pointers/values in place), while `update_array_*` /
  `update_scalar_*` intentionally act only on the pointer/value contents and
  don't need a reference.
- `include/private/memory_tracker.hpp` + `src/memory_tracker.cpp` — free
  functions (`add_to_memory_tracker`, `remove_from_memory_tracker`) that
  maintain a `std::map<void*, std::tuple<label, size, on_device>>` plus a
  running `(host_bytes, device_bytes)` total. This tracker is the source of
  truth `DualMemoryManager` uses to detect double-free/untracked-free bugs and
  to produce usage reports (`src/memory_usage.cpp`,
  `return_total_memory_usage()` / `report_memory_usage()`).
- `include/private/abort.hpp` + `src/abort.cpp` — `MiMMO::abort_mimmo()`,
  the single error-handling path used everywhere invalid state is detected
  (null pointers, tracking failures). There are no exceptions in this
  codebase; invalid state aborts the process.
- Three macros bridge the OpenACC/CPU code paths inside compute regions and
  pragmas, since a `DualArray`/`DualScalar` always carries *both* a host and
  device representation. Macros are preprocessor text, not C++ symbols, so
  they are defined outside `namespace MiMMO` and are `MIMMO_`-prefixed
  instead of namespaced:
  - `MIMMO_GET_PTR(x)` / `MIMMO_GET_VALUE(x)` — resolve to the device or host
    pointer/value depending on whether `_OPENACC` is defined; use only inside
    parallel regions.
  - `MIMMO_PRESENT(x)` — expands to `copyin(x) deviceptr(x.dev_ptr)` (or
    nothing without OpenACC); used inside `#pragma acc` clauses to tell
    OpenACC the struct's device pointer is already valid, since MiMMO manages
    device memory manually rather than letting OpenACC's runtime move it.
  All `DualMemoryManager` methods must only be called from host code; device
  code should only ever touch data through the macros.
- `examples/` contains two worked patterns: `scalar_product/` (plain
  host/device array + scalar usage) and `globals/` (using `DualScalar` to
  manage `extern` global variables shared between host and device, since
  OpenACC only auto-manages *local* variables).

## Code style

- Formatting is enforced by `.clang-format` (2-space indent, 80-column limit)
  and applied automatically via the pre-commit hook
  (`pre-commit run --all-files` or just `git commit` if hooks are installed).
- Naming: `PascalCase` for classes/structs, `snake_case` for functions and
  variables, `UPPER_SNAKE_CASE` for macros/constants. Classes, functions, and
  variables live inside the `MiMMO` namespace (no suffixing); macros cannot be
  namespaced in C++, so they instead use a `MIMMO_` prefix (e.g.
  `MIMMO_GET_PTR`) and are declared outside `namespace MiMMO`.
- Headers use `#pragma once`. Public API lives under `include/mimmo/`;
  implementation details (including templated `.inl` files) live under
  `include/private/`.
- Public APIs use Doxygen comments (`@brief`, `@details`, `@tparam`, `@see`).
- Errors are handled by calling `MiMMO::abort_mimmo()`, not by throwing.
