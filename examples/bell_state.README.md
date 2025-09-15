# Bell State Example

## Overview
`bell_state.qpp` builds a two-qubit circuit, entangles the qubits with a Hadamard and CNOT, and measures both qubits using the `qpp::api::Program` helper around `LocalSimBackend`.

## Key Concepts Demonstrated
- Allocating qubits, appending gates, and recording measurements through `qpp::api::Circuit`.
- Executing the circuit on the built-in deterministic simulator and iterating over the returned probability table.

## Build and Run
### CMake (recommended)
1. Configure and build the examples (see `examples/README.md` for full context):
   ```sh
   cd examples
   mkdir -p build && cd build
   cmake ..
   make bell_state
   ```
2. Execute the sample from the build directory:
   ```sh
   ./bell_state
   ```
   The output lists measurement probabilities, e.g. `00: 0.5` and `11: 0.5` for the ideal Bell state.

### Stand-alone Compilation
Compile the `.cpp` wrapper together with the simulator implementation from the repository root:
```sh
g++ -std=c++17 -Iinclude -I. examples/bell_state.cpp \
    qpp/backend/LocalSimBackend.cpp -o bell_state
./bell_state
```
The wrapper forwards to `bell_state.qpp`, so including the headers from `include/` is sufficient for the rest of the Q++ API.
