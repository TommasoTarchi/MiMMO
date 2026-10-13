# MiMMO

[![License: GPL-3.0](https://img.shields.io/badge/License-GPL--3.0-green.svg)](https://www.gnu.org/licenses/gpl-3.0.en.html)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![Documentation](https://img.shields.io/badge/docs-GitHub%20Pages-blue.svg)](https://tommasotarchi.github.io/MiMMO/)
[![OpenACC](https://img.shields.io/badge/OpenACC-2.0-15B2D3?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgdmlld0JveD0iMCAwIDI0IDI0IiBmaWxsPSJub25lIiBzdHJva2U9IiMxNUIyRDMiIHN0cm9rZS13aWR0aD0iMiI+PHBhdGggZD0iTTEyIDJDNi40OCAyIDIgNi40OCAyIDEyczQuNDggMTAgMTAgMTAgMTAtNC40OCAxMC0xMFMxNy41MiAyIDEyIDJ6bTAgMTRjLTUuNTIgMC0xMC00LjQ4LTEwLTEwUzYuNDggMiAxMiAyUzIyIDYuNDggMjIgMTJzLTQuNDggNi00LjQ4IDZoLS40OEw3IDE0bDQgNCA0LTRMMiA1LjI4WiIgLz48L3N2Zz4=)](https://www.openacc.org/)

**MiMMO** (Minimal Memory Manager for OpenACC) is a simple, safe, and
easy-to-use CPU/GPU memory manager for OpenACC programs.

OpenACC's pragma-based approach is powerful, but its high-level memory
management can be opaque. MiMMO provides finer control over memory
allocations and transfers while keeping the code within compute regions
clean and safe.

Under the hood, MiMMO wraps the OpenACC runtime API. It tracks all host
and device allocations, with memory reports available on demand.

## Features

- **Dual arrays**: Explicit host/device pointers with tracked metadata
- **Dual scalars**: Explicit management for global `extern` variables
  (local variables are handled automatically by OpenACC)
- **Safety**: Macros ensure correct pointer access within parallel regions
- **Transparency**: Request memory usage reports at any time
- **Header-only device layer**: all OpenACC calls live in templated
  headers, instantiated in *your* translation units, so GPU support is
  controlled by how your program is compiled, not by how MiMMO itself
  was built

## Table of Contents
- [Quick Start](#quick-start)
- [Requirements](#requirements)
- [Building and Testing](#building-and-testing)
- [Documentation](#documentation)
- [Usage](#usage)
- [API Reference](#api-reference)
- [Contributing](#contributing)
- [License](#license)

## Quick Start

```bash
# Clone and build
git clone git@github.com:TommasoTarchi/MiMMO.git
cd MiMMO
mkdir build && cmake -S . -B build && cmake --build build

# Run tests
ctest --test-dir build --output-on-failure
```

Include `mimmo/api.hpp` and link against `libmimmo.so` (in `build/`).

## Requirements

- **CMake**: ≥ 3.15
- **C++17 compiler**: **NVIDIA HPC SDK (NVHPC)** 25.x recommended
- **Doxygen** (optional): for API documentation

> **Note**: MiMMO's device-memory logic (`acc_malloc`,
> `acc_memcpy_to_device`, ...) lives in templated headers, so it is
> compiled into *your* program, not into `libmimmo.so`. GPU support is
> therefore controlled by whether your own translation units are
> compiled with `-acc`, regardless of the flags MiMMO itself was built
> with — a `libmimmo.so` built with `-DTESTS=CPU` is just as GPU-capable
> as one built with NVHPC.
>
> `-DTESTS=CPU` (or `-DTESTS=NONE`) is required on non-NVHPC compilers
> (the default, `GPU`, requires `CXX=nvc++` and aborts configuration
> otherwise).

## Building and Testing

```bash
mkdir build
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Options:
- `-DTESTS=NONE|CPU|GPU` (default `GPU`): Which tests to build. `NONE`
  skips the Catch2 test target entirely; `CPU` builds host-only tests on
  any C++17 compiler; `GPU` builds full OpenACC tests and requires NVHPC
  (`CXX=nvc++`)

## Documentation

The API documentation is published at
[tommasotarchi.github.io/MiMMO](https://tommasotarchi.github.io/MiMMO/),
and is rebuilt automatically whenever `main` is updated.

To generate it locally (requires Doxygen, and Graphviz for the diagrams):

```bash
cmake --build build --target docs
```

Docs are placed in `build/docs/html/`; open `build/docs/html/index.html`
in a browser.

## Usage

MiMMO works with *dual arrays* (host/device pointers with metadata) and
*dual scalars* (host value with device pointer for global `extern`
variables). Include `mimmo/api.hpp` and use the `MiMMO` namespace.

Working examples are in [`examples/`](./examples/).

### Compile your program

```bash
nvc++ -acc -c my_program.cpp -I</path/to/MiMMO>/include
nvc++ -acc my_program.o -L</path/to/MiMMO>/build -lmimmo -o my_program
```

Ensure the dynamic linker finds the library:
```bash
export LD_LIBRARY_PATH=</path/to/MiMMO>/build:$LD_LIBRARY_PATH
```

### CPU-only mode

If your program is compiled without `-acc` (so `_OPENACC` is undefined),
MiMMO falls back to a host-only mode instead of failing:

- `DualArray::dev_ptr` / `DualScalar::dev_ptr` stay `nullptr`, even when
  `on_device` is `true`
- `update_array_*` / `update_scalar_*` become no-ops
- `MIMMO_GET_PTR()` / `MIMMO_GET_VALUE()` resolve to the host
  pointer/value, and `MIMMO_PRESENT()` expands to nothing

This lets the same source compile and run correctly as a plain host
memory manager on machines without OpenACC.

## API Reference

See the [online documentation](https://tommasotarchi.github.io/MiMMO/)
for full details.

### Data structures

- **`DualArray`**: Contains `host_ptr`, `dev_ptr`, `size`, `size_bytes`
- **`DualScalar`**: Contains `host_value`, `dev_ptr`

### Class

- **`DualMemoryManager`**: Memory manager with methods for allocating,
  copying, and freeing dual arrays and scalars
  - Arrays: `alloc_array()`, `update_array_host_to_device()`,
    `update_array_device_to_host()`, `free_array()`
  - Scalars: `create_scalar()`, `update_scalar_host_to_device()`,
    `update_scalar_device_to_host()`, `destroy_scalar()`
  - Reporting: `return_total_memory_usage()`, `report_memory_usage()`

> All `DualMemoryManager` methods must be called from the host only.

### Macros

- **`MIMMO_GET_PTR()`**: Returns device pointer (OpenACC) or host
  pointer; **use inside parallel regions only**
- **`MIMMO_GET_VALUE()`**: Returns device value (OpenACC) or host value;
  **use inside parallel regions only**
- **`MIMMO_PRESENT()`**: Informs OpenACC that data is already on device;
  use in pragma clauses

> Always use `MIMMO_PRESENT()` in pragmas to indicate data is present on
> device.

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md)
for guidelines.

## License

GNU General Public License v3.0. See [LICENSE](LICENSE).

## Citation

```
Tarchi, T. (2026). MiMMO: Minimal Memory Manager for OpenACC. 
GitHub. https://github.com/TommasoTarchi/MiMMO
```
