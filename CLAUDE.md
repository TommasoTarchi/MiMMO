# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

@AGENTS.md

## Architecture

- The entire public API is one header, `include/mimmo/api.hpp`. It declares `DualArray<T>`,
  `DualScalar<T>`, and `DualMemoryManager` together (they are not split into separate
  `dual_array.hpp` / `dual_scalar.hpp` files despite what the naming-convention examples in
  AGENTS.md might suggest), then `#include`s the template method bodies from
  `include/private/arrays.inl` (array alloc/copy/free) and `include/private/scalars.inl`
  (scalar create/copy/destroy).
- The library is *not* fully header-only in practice: `src/abort.cpp`, `src/memory_tracker.cpp`,
  and `src/memory_usage.cpp` are non-template translation units compiled into `libmimmo.so`.
  They back `include/private/abort.hpp` (`MiMMO::abort_mimmo`, the sole error path — no
  exceptions), `include/private/memory_tracker.hpp` (free functions maintaining the manager's
  internal `std::map<void*, std::tuple<std::string,size_t,bool>>` allocation registry), and
  `DualMemoryManager::return_total_memory_usage()` / `report_memory_usage()`. Only the templated
  array/scalar operations are header-only.
- `MIMMO_GET_PTR()`, `MIMMO_GET_VALUE()`, and `MIMMO_PRESENT()` (defined in `api.hpp`) each branch
  on `#ifdef _OPENACC`: with OpenACC they resolve to the device pointer/value and a
  `deviceptr`/`copyin` clause; without it, to the host pointer/value and an empty clause. The two
  branches must stay in sync — this is the main invariant `.claude/rules/openacc.md` exists to
  protect, and why tests mirror the same `#ifdef` split per `TEST_CASE`.
- Root `CMakeLists.txt` is the only CMake file in the repo (no sub-`CMakeLists.txt` under `src/`,
  `tests/`, or `examples/`). With `-DOPENACC=ON` (default) it requires
  `CMAKE_CXX_COMPILER_ID MATCHES "NVHPC"` and calls `find_package(OpenACC REQUIRED)`. With
  `-DUNIT_TESTS=ON` (default) it `FetchContent`s Catch2 v3.4.0 to build `tests/unit_tests_main.cpp`
  into `build/unit_tests.x`. The `docs` target (Doxygen, config in `docs/Doxyfile.in`) is skipped
  with a warning rather than failing the configure step when Doxygen isn't installed.
- `examples/` (`scalar_product/plain/`, `globals/`) are documentation, not build targets: each has
  a `compile_and_run.txt` with manual `nvc++ -acc` compile/link steps against the built
  `libmimmo.so`, and nothing under `examples/` is wired into CMake or CI.
- CI (`.github/workflows/ci.yaml`) always configures with `-DOPENACC=OFF` (no GPU runner), and runs
  a `clang-format` check via `pre-commit/action` before building/testing — so a formatting-only
  failure there is unrelated to OpenACC.

## Claude Code specifics

- Build artifact is `build/libmimmo.so`; the public entry point is `include/mimmo/api.hpp`.
  Consumers compile with `nvc++ -acc -I<repo>/include` and link `-L<repo>/build -lmimmo`.
- Run the narrowest test that covers a change first:
  `ctest --test-dir build -R <test_name> --output-on-failure`. Run the full suite before
  declaring a change finished.
- When `nvc++`/NVHPC is unavailable, configure with `-DOPENACC=OFF` so the non-OpenACC path
  still builds and tests. A change is not verified until both configurations build.
- Read `CONTRIBUTING.md` before proposing changes to project structure or conventions.
- Detailed rules for dual-memory and OpenACC code live in `.claude/rules/openacc.md` and
  load automatically when you touch matching files.
- Git usage is blocked at the tool level (see `.claude/settings.json`), not just requested
  in AGENTS.md, so it's enforced even if Claude forgets the instruction. The `git clone`
  line in `README.md` is instructions for humans, not something to execute.
- The sandboxed Bash tool is enabled and restricted to the project directory (see
  `.claude/settings.json`). Enforced by the OS, not by instructions, so it holds for
  subagents too.
