# MiMMO

[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![OpenACC](https://img.shields.io/badge/OpenACC-2.0-15B2D3?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgdmlld0JveD0iMCAwIDI0IDI0IiBmaWxsPSJub25lIiBzdHJva2U9IiMxNUIyRDMiIHN0cm9rZS13aWR0aD0iMiI+PHBhdGggZD0iTTEyIDJDNi40OCAyIDIgNi40OCAyIDEyczQuNDggMTAgMTAgMTAgMTAtNC40OCAxMC0xMFMxNy41MiAyIDEyIDJ6bTAgMTRjLTUuNTIgMC0xMC00LjQ4LTEwLTEwUzYuNDggMiAxMiAyUzIyIDYuNDggMjIgMTJzLTQuNDggNi00LjQ4IDZoLS40OEw3IDE0bDQgNCA0LTRMMiA1LjI4WiIgLz48L3N2Zz4=)](https://www.openacc.org/)

**MiMMO** (Minimal Memory Manager for OpenACC) is a simple, safe, and easy-to-use CPU/GPU memory manager for OpenACC programs.

OpenACC's pragma-based approach is powerful, but its high-level memory management can be opaque. MiMMO provides finer control over memory allocations and transfers while keeping the code clean and safe.

Under the hood, MiMMO wraps the OpenACC runtime API. It tracks all host and device allocations, with memory reports available on demand.

## Features

- **Dual arrays**: Explicit host/device pointers with tracked metadata
- **Dual scalars**: Simplified global variable management
- **Safety**: Macros ensure correct pointer access within parallel regions
- **Transparency**: Request memory usage reports at any time
- **Header-only core**: GPU support only requires linking OpenACC at compile time

## Table of Contents
- [Quick Start](#quick-start)
- [Requirements](#requirements)
- [Building and Testing](#building-and-testing)
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

> **Note**: GPU support is not required to compile MiMMO. The library is header-only for OpenACC calls; GPU features activate by linking OpenACC at compile time.
>
> Use the same compiler for MiMMO and your program. On non-OpenACC compilers, use `-DOPENACC=OFF` or `-DUNIT_TESTS=OFF`.

## Building and Testing

```bash
mkdir build
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Options:
- `-DUNIT_TESTS=OFF`: Skip Catch2 test overhead
- `-DOPENACC=OFF`: Build without OpenACC support

### Generate documentation

```bash
cmake --build build --target docs
```

Docs are placed in `build/docs/`.

## Usage

MiMMO works with *dual arrays* (host/device pointers with metadata) and *dual scalars* (host value with device pointer). Include `mimmo/api.hpp` and use the `MiMMO` namespace.

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

## API Reference

See the [Doxygen documentation](#generate-documentation) for full details.

### Data structures

- **`DualArray`**: Contains `host_ptr`, `dev_ptr`, `label`, `size`, `size_bytes`
- **`DualScalar`**: Contains `host_value`, `dev_ptr`, `label`

### Class

- **`DualMemoryManager`**: Memory manager with methods for allocating, copying, and freeing dual arrays and scalars
  - Arrays: `alloc_array()`, `update_array_host_to_device()`, `update_array_device_to_host()`, `free_array()`
  - Scalars: `create_scalar()`, `update_scalar_host_to_device()`, `update_scalar_device_to_host()`, `destroy_scalar()`
  - Reporting: `return_total_memory_usage()`, `report_memory_usage()`

> All `DualMemoryManager` methods must be called from the host only.

### Macros

- **`MIMMO_GET_PTR()`**: Returns device pointer (OpenACC) or host pointer; use inside parallel regions only
- **`MIMMO_GET_VALUE()`**: Returns device value (OpenACC) or host value; use inside parallel regions only
- **`MIMMO_PRESENT()`**: Informs OpenACC that data is already on device; use in pragma clauses

> Always use `MIMMO_PRESENT()` in pragmas to indicate data is present on device.

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## License

GNU General Public License v3.0. See [LICENSE](LICENSE).

## Citation

```
Tarchi, T. (2026). MiMMO: Minimal Memory Manager for OpenACC. 
GitHub. https://github.com/TommasoTarchi/MiMMO
```
