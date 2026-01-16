# MiMMO

**WARNING**: This library is still under development, and many of its features are still to be tested.

**WARNING**: Compilation for GPU is not available with CMake yet, it will be soon.

**WARNING**: This README is still to be completed.

**MiMMO** (Minimal Memory Manager for Openacc) is a simple, safe, easy to use CPU/GPU memory manager to
work with OpenACC.

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

TODO

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
   Since testing is enabled by default, the tests will also be built. If you want to avoid
   Catch2 overhead in the building phase, you can disable tests by adding `-DUNIT_TESTS=OFF`
   to the `cmake -S . -B build` command.

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

The library works with so called "dual arrays", i.e. data structures containing host and device
pointers of the array, in addition to the number of elements, the size in bytes and the label used
by the memory manager to track the array.

To use MiMMO in your C++ project, include the header `mimmo/api.hpp` and access the functions within
the `MiMMO` namespace (macros, instead, always begin with `MIMMO_`). Here's a simple example:

TODO: add example

Compile your program linking against the MiMMO library as follows, adjusting the paths as necessary:
```bash
g++ -c my_program.cpp -I</path/to/MiMMO>/include
g++ my_program.o -L</path/to/MiMMO>/build -lmimmo -o my_program
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
  - `host_ptr`: pointer to array memory on host;
  - `dev_ptr`: pointer to array memory on device;
  - `label`: label used to track dual array memory;
  - `dim`: number of elements in array;
  - `size`: size of array in bytes.

### Classes

- `DualMemoryManager`: memory manager that can be used to manage dual arrays; the following methods
  are made available:
  - `allocate()`: allocate requested memory on host and (optionally) on device;
  - `copy_host_to_device()`: copy array data from host to device;
  - `copy_device_to_host()`: copy array data from device to host;
  - `free()`: free both host and device memory;
  - `report_memory_usage()`: print report of memory usage for both host and device.

### Helper macros

- `MIMMO_GET_PTR()`: returns the device pointer of a dual array if OpenACC is enabled, and the host
  pointer otherwise; it is thought to be used inside OpenACC compute regions.
- `MIMMO_GET_DIM()`: returns the dimension (i.e. number of elements) of a dual array.
- `MIMMO_PRESENT()`: communicates inside a pragma decorating a compute region that the data of a
  dual array are already present on device; also copies dimension of array to device.

## Contributing

Contributions are welcome!

If you find a bug, have an idea for an improvement, or want to add a new
feature, feel free to open an issue or submit a pull request.

## License

This project is licensed under the GNU General Public License v3.0.
See the [LICENSE](LICENSE) file for details.
