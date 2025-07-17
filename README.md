# Q++ GCC

This repository is a fork of GCC with experimental extensions for quantum programming.
For background information on GCC itself see [README](README). The additions here focus on
new language constructs and tooling to support quantum-aware C++ development.

## Q++ Highlights

- **`qstruct` and `qclass`** &mdash; Data structures for quantum state
  storage and manipulation.
- **Probabilistic `bool`** &mdash; Boolean values that represent
  quantum superposition probabilities.
- **Bitwise gate macros** &mdash; Convenience wrappers for common
  quantum logic gates such as `H`, `X`, `CNOT` and others.
- **Hardware API stubs** &mdash; Placeholder interfaces intended for
  integration with future quantum devices.

These features are experimental and not part of upstream GCC.

## Building with CMake

The workflow mirrors the process referenced in
[CONTRIBUTING](.github/CONTRIBUTING.md) but uses CMake for configuration.
A typical build looks like the following:

```sh
# Create a separate build directory
mkdir build && cd build

# Configure
cmake ..

# Compile
cmake --build .

# Run the test suite
ctest

# Optionally install
cmake --install .
```

Refer to the upstream GCC README for additional information about the
compiler and licensing.

