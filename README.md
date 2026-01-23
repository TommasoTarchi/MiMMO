# MiMMO

**MiMMO** (Minimal Memory Manager for Openacc) is a simple, safe, easy to use CPU/GPU memory manager to
work with OpenACC ([https://www.openacc.org/](https://www.openacc.org/)) in C++.

The pragma-based approach of OpenACC is extremely useful, however it only allows for a very high-level
management of host and device memory, and it is often confusing for those who like to understand what is
happening behind the curtain.

MiMMO aims to allow the users to have a finer control over memory allocations and movements, while keeping
it simple and safe.

Under the hood, MiMMO uses the OpenACC runtime API; however, it hides it completely, allowing the user to
write clean code for both host and device.

The manager also keeps track of all memory allocations on host and device, and reports can be requested at
any time.

## Table of Contents
- [Requirements](#requirements)
- [Building and Testing](#building-and-testing)
- [Usage](#usage)
- [API Reference](#api-reference)
- [Contributing](#contributing)
- [License](#license)

## Requirements

- **CMake**: at least version 3.15
- **Compiler supporting C++17**: recommended **NVIDIA HPC SDK (NVHPC)** version 25.x
- **Doxygen** (optional: required to generate API documentation)

### Notes on compiler

Even if the library is thought to be used inside OpenACC programs, having GPU support is not required
during its compilation.

In fact, all parts of the library involving OpenACC calls are header-only; i.e. to use them with GPU
capabilities you only need to link OpenACC while compiling your own program.

The only part of the library that is compiled differently depending on whether OpenACC is enabled are the
tests. If you are using a compiler that does not support OpenACC, please compile without GPU support
(using `-DOPENACC=OFF`) or disable tests completely (using `-DUNIT_TESTS=OFF`).

In any case, we suggest to use the same compiler to compile the library and the program including it. In
particular we suggest to use **NVIDIA HPC SDK (NVHPC)** at least version 21.7 (recommended 25.x).

## Building and Testing

MiMMO can be compiled as a shared library, using CMake.

To build the library and run tests, follow these steps:

1. Clone the repository:
   ```bash
   git clone git@github.com:TommasoTarchi/MiMMO.git
   cd MiMMO
   ```

2. Create a build directory and compile the code:
   ```bash
   mkdir build
   cmake -S . -B build
   cmake --build build
   ```
   Notice:
   - Testing is enabled by default. If you want to avoid Catch2 overhead in the building phase, you can
   disable tests by adding `-DUNIT_TESTS=OFF` to the `cmake -S . -B build` command.
   - OpenACC in tests is enabled by default. If you want to disable it, you can add `-DOPENACC=OFF` to
   the `cmake -S . -B build` command.

3. (Optional) Run the tests:
   ```bash
   ctest --test-dir build --output-on-failure
   ```

The compiled library `libmimmo.so` will be located in the `build/` directory.

### Generate documentation

We recommend to generate Doxygen documentation as well.

This can be done, after the library has been compiled, by issuing:
```bash
cmake --build build --target docs
```

The generated documentation will be placed in `build/docs/`.

## Usage

The library works with so called *dual arrays*, i.e. data structures containing host and device
pointers of the array, in addition to the number of elements, the size in bytes and the label used
by the memory manager to track the array.

To use MiMMO in your C++ project, include the header `mimmo/api.hpp` and access the functions within
the `MiMMO` namespace; macros, instead, always begin with `MIMMO_`.

Examples of working OpenACC programs using MiMMO can be found in the [examples folder](./examples/).

Compile your program linking against the MiMMO library as follows, adjusting the paths as needed:
```bash
nvc++ -acc -Minfo=all -c my_program.cpp -I</path/to/MiMMO>/include
nvc++ -acc -Minfo=all my_program.o -L</path/to/MiMMO>/build -lmimmo -o my_program
```

Before running your program, ensure that the dynamic linker can find the `libmimmo.so` library. You
can set the `LD_LIBRARY_PATH` environment variable to include the directory where the library is
located:
```bash
export LD_LIBRARY_PATH=</path/to/MiMMO>/build:$LD_LIBRARY_PATH
```

## API Reference

The following is a list of all features available in the MiMMO API. For a more detailed description
of them, please see the Doxygen documentation (see [Generate documentation](#generate-documentation)
for how to generate it).

### Data structures

- `DualArray`: dual array data structure; it contains the following fields:
  - `host_ptr`: pointer to allocated memory on host;
  - `dev_ptr`: pointer to allocated memory on device;
  - `label`: label used to track dual array memory;
  - `size`: number of elements in array;
  - `size_bytes`: size of array in bytes.

### Classes

- `DualMemoryManager`: memory manager that can be used to manage dual arrays; the following methods
  are currently available:
  - `allocate()`: allocates the requested memory on host and (optionally) on device;
  - `copy_host_to_device()`: copies the array data from host to device;
  - `copy_device_to_host()`: copies the array data from device to host;
  - `free()`: frees both host and device memory;
  - `return_total_memory_usage()`: returns the total amount of memory used on host and device;
  - `report_memory_usage()`: prints a report of memory usage for both host and device.

### Helper macros

- `MIMMO_GET_PTR()`: if the program was compiled with OpenACC it returns the device pointer, otherwise
  the host pointer of a given dual array; for safety, it should only be used inside OpenACC parallel
  regions.
- `MIMMO_PRESENT()`: communicates inside a pragma decorating an OpenACC parallel region that the data
  of a dual array are already present on device; only to be used in OpenACC pragmas.

## Contributing

Contributions are welcome!

If you find a bug, have an idea for an improvement, or want to add a new feature, feel free to open an
issue or submit a pull request.

## License

This project is licensed under the GNU General Public License v3.0. See [LICENSE](LICENSE) for details.

If you use this library in your work, please consider citing the repository:
````
Tarchi, T. (2026). MiMMO (Minimal Memory Manager for Openacc): a simple, safe, easy to use CPU/GPU
memory manager to work with OpenACC. GitHub repository. https://github.com/TommasoTarchi/MiMMO
````
