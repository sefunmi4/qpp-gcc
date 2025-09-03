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
