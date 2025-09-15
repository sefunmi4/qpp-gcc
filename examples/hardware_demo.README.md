# Hardware Backend Demo

## Overview
`hardware_demo.qpp` compares executing a simple Bell-state circuit on either a discovered hardware backend or the local simulator. It requests a backend via `QPUBackend::discoverDevices()`, falls back to `LocalSimBackend` when no device is found, and reports both execution time and throughput for the selected backend and simulator.

## Execution Flow
1. Build a two-qubit circuit with Hadamard, CNOT, and measurement, configured for 1024 shots.
2. Inspect `QPU_PCI_DEVICE`, `QPU_USB_DEVICE`, and `QPU_NET_DEVICE` environment variables to choose a hardware device.
3. Run the program on the chosen backend, measure wall-clock duration, then repeat with `LocalSimBackend` for comparison.

## Build and Run
### CMake
```sh
cd examples
mkdir -p build && cd build
cmake ..
make hardware_demo
./hardware_demo
```
Set one of the `QPU_*_DEVICE` variables (and optionally `QPU_AUTH_TOKEN`) to steer hardware selection. Without a device, the program automatically reports the simulator fallback.

### Stand-alone Compilation
Compile from the repository root, linking both backend implementations:
```sh
g++ -std=c++17 -Iinclude -I. examples/hardware_demo.cpp \
    qpp/backend/LocalSimBackend.cpp qpp/backend/QPUBackend.cpp -o hardware_demo
./hardware_demo
```
