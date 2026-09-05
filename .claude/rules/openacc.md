---
paths:
  - "include/**/*.{hpp,inl}"
  - "src/**/*.{cpp,hpp}"
  - "tests/**/*.cpp"
  - "examples/**/*.cpp"
---

# Dual memory and OpenACC rules

Applies whenever you edit code touching `DualMemoryManager`, `DualArray`, `DualScalar`,
or any `#pragma acc` region.

## Accessor macros

- `MIMMO_GET_PTR()` and `MIMMO_GET_VALUE()` are for use **inside compute regions only**.
  Outside a parallel region, use `host_ptr` / `host_value` directly. Using the accessors
  on the host, or the raw members inside a compute region, is the most common bug here.
- Always include `MIMMO_PRESENT()` in `#pragma acc` clauses for data already on the device.
  Omitting it makes OpenACC re-manage memory MiMMO already tracks.

## Data structures

- `DualScalar` exists for **global `extern` variables only**. Local scalars are handled
  automatically by OpenACC — don't wrap them.

## Manager API

- Host context only. Never call a `DualMemoryManager` method from device code or an
  `acc routine`.
- Array and scalar lifecycles use different verbs; don't mix them up:
  - arrays: `alloc_array()` / `free_array()`,
    `update_array_host_to_device()` / `update_array_device_to_host()`
  - scalars: `create_scalar()` / `destroy_scalar()`,
    `update_scalar_host_to_device()` / `update_scalar_device_to_host()`
- Every alloc/create needs a matching free/destroy. Verify with
  `return_total_memory_usage()` or `report_memory_usage()` rather than by inspection.

## Code organisation

- Core is header-only: public headers in `include/mimmo/`, private headers and `.inl`
  template implementations in `include/private/`, instantiated via `api.hpp`.
- Guard device-specific code with `#ifdef _OPENACC` and update **both** branches together.
  Don't patch only the path you can compile locally.
- Errors go through `MiMMO::abort()` (`include/private/abort.hpp`), not exceptions or
  raw `std::abort`.

## Tests

- Catch2, tagged `[mimmo]`, in the `unit_tests.x` target.
- Cover both the OpenACC and non-OpenACC path, and assert on memory tracking so leaks
  in new alloc/free pairs are caught.
