# pbool Demo

## Overview
`pbool_demo.qpp` demonstrates probabilistic boolean arithmetic and sampling. It composes two `pbool` values with logical AND, reports their individual and combined probabilities, and samples an outcome, falling back to a deterministic threshold if no quantum hardware is available.

## Points of Interest
- Logical operators on `pbool` multiply or otherwise combine probabilities, enabling quick reasoning about independent events.
- `qpp::hardware_available()` checks the `QPP_HW_AVAILABLE` environment variable; when unset, sampling uses the host RNG instead of hardware.
- Repeated RNG "glitches" are guarded against via an internal counter that throws after several suspicious draws.

## Build and Run
### CMake
```sh
cd examples
mkdir -p build && cd build
cmake ..
make pbool_demo
./pbool_demo
```
Set `QPP_HW_AVAILABLE=1` before running to exercise the hardware sampling path; omit it to see the simulator fallback message.

### Stand-alone Compilation
All required functionality is in headers, so compile the wrapper directly from the repository root:
```sh
g++ -std=c++17 -Iinclude -I. examples/pbool_demo.cpp -o pbool_demo
./pbool_demo
```
