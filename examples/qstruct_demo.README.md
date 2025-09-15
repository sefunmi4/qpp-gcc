# qstruct Demo

## Overview
`qstruct_demo.qpp` highlights the low-level `qclass` container that wraps a `qstruct` state vector. The sample initializes a single-qubit register, applies a Hadamard followed by a Pauli-X, and prints the resulting amplitudes.

## Concepts Covered
- Direct gate application on the in-memory quantum state without invoking a backend.
- Accessing the internal amplitude vector for inspection or custom post-processing.

## Build and Run
### CMake
```sh
cd examples
mkdir -p build && cd build
cmake ..
make qstruct_demo
./qstruct_demo
```
The amplitudes printed after applying `H` then `X` illustrate how the state vector is updated in place (both |0⟩ and |1⟩ amplitudes remain at 1/√2).

### Stand-alone Compilation
The demo is header-only and can be built directly from the repository root:
```sh
g++ -std=c++17 -Iinclude -I. examples/qstruct_demo.cpp -o qstruct_demo
./qstruct_demo
```
