# Hardware API Stub Example

## Overview
`hardware_api_example.qpp` exercises the lightweight `HardwareStub` implementation to show how Q++ code can emit Quantum Intermediate Representation (QIR) instructions before actual device connectors are available.

## Walkthrough
- Instantiate `qpp::HardwareStub` inside `main`.
- Emit a sequence of textual operations (`H`, `CX`, and `MEASURE`) that the stub records in memory.
- Print the buffered program to standard output so you can inspect the generated QIR payload.

## Build and Run
### CMake
```sh
cd examples
mkdir -p build && cd build
cmake ..
make hardware_api_example
./hardware_api_example
```
The program prints the collected instructions separated by newlines.

### Stand-alone Compilation
When building from the repository root, only the headers are required:
```sh
g++ -std=c++17 -Iinclude -I. examples/hardware_api_example.cpp -o hardware_api_example
./hardware_api_example
```
