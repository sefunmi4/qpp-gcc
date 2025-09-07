# Hardware API

This file sketches the interface expected by future Q++ hardware
backends. A hardware profile is described in a simple YAML format and
defines qubit count, supported gates and rate limits.

The runtime emits QIR strings when executing tasks. Real backends would
translate this intermediate form to device specific commands. For now
`HardwareStub` simply stores the strings for inspection.

The compiler attaches probability metadata to the emitted QIR so that
hardware schedulers can make informed decisions.  Hardware profiles
describe qubit counts, supported gates and rate limits using YAML.  A
priority-aware scheduler may pause, resume, reprioritize or remove
tasks based on these profiles.

The API is intentionally minimal:

```cpp
class HardwareStub {
  void emit(const std::string& qir);
  std::string result() const;
};
```

Implementations may choose to stream operations, perform buffering or
apply other optimizations. Rate-limit handling and multi-backend support
are left for future work.


## Initial Mappers

Logical qubits must be assigned to physical locations before routing.  The
`AbstractInitialMapper` interface encapsulates this placement step. Two
implementations are provided:

* `LineInitialMapper` – maps qubits along a simple device line, centering the
  highest-degree logical nodes.
* `VF2InitialMapper` – explores placements using a pruned backtracking search
  similar to the VF2 algorithm.  If no complete mapping is found within the
  time budget it falls back to the line heuristic.

A `DynamicInitialMapper` chooses between these strategies based on circuit
size and interaction density and serves as the default when no mapper is
specified.

## External Providers

Basic support for running programs on external hardware APIs is provided
through the :class:`HardwareAPI` helper in ``hardware_api.py``.  It can
connect to either Qiskit or Cirq backends.  Credentials may be supplied
using ``add_credentials`` (the earlier ``add_credientials`` spelling is
still accepted for backward compatibility):

```
from hardware_api import HardwareAPI
api = HardwareAPI('qiskit')
api.add_credentials('qiskit', token='MYTOKEN')
backend = api.connect()
```

If no credentials are provided the helper assumes that a backend is
available locally and falls back to the frameworks' simulators.  This
mirrors execution on a classical wavefunction simulator or a directly
attached QPU.

To run on a specific device, call ``select_backend`` with the desired
backend name.  For example, ``"statevector_simulator"`` chooses the
statevector simulator in Qiskit while ``"density_matrix"`` selects the
matching Cirq simulator.

### Running C++/Q++ Programs

Q++ or C++ sources are lowered to OpenQASM before execution.  The
resulting string can be submitted to an external provider using
``run_qasm`` or ``run_source``.  ``run_qasm`` accepts a pre-generated
OpenQASM string while ``run_source`` compiles a Q++/C++ file containing
``__qasm`` blocks or ``circuit.push_back`` statements and executes the
resulting program:

```
from hardware_api import HardwareAPI

api = HardwareAPI('cirq')
api.select_backend('density_matrix')
result = api.run_source('program.qpp')[0]
```

Both helpers execute the program on the configured backend, falling back
to local simulators when credentials are omitted.

### Example Q++ Source

An end-to-end example Q++ file demonstrates how the helper can be used
from C++/Q++ code. Circuits in Q++ are vectors of gate operations, so the
`Circuit` type inherits from `std::vector` and accepts gates via
`push_back`. The program below targets Qiskit and can be switched to
Cirq by changing the provider name:

```cpp
#include "hardware_api.hpp"

int main() {
  // Replace "qiskit" with "cirq" to target Cirq instead
  HardwareAPI api("qiskit");
  api.select_backend("statevector_simulator");

  Circuit circuit; // Circuit inherits from std::vector
  circuit.push_back(H(0));
  circuit.push_back(CX(0,1));
  circuit.push_back(Measure(0,1));

  return 0;
}
```

Save the file as ``examples/hardware_api_example.qpp`` and execute it
using ``run_source``:

```
from hardware_api import HardwareAPI
api = HardwareAPI('qiskit')
api.run_source('examples/hardware_api_example.qpp')
```

### Testing

The connectors can be exercised with the provided unit tests.  After
installing optional dependencies (``pip install qiskit cirq``) run:

```
pytest tests/test_hardware_api.py
```

The tests automatically skip when the required frameworks are not
installed, allowing them to run in minimal environments.
