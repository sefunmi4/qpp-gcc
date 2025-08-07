# Q++ GCC

This repository is a fork of GCC with experimental extensions for quantum programming.
For background information on GCC itself see [README](README). The additions here focus on
new language constructs and tooling to support quantum-aware C++ development.

## Q++ Highlights

- **`qstruct` and `qclass`** &mdash; Data structures for quantum state
  storage and manipulation.  See
  [`include/qpp/qstruct.hpp`](include/qpp/qstruct.hpp) for
  reference implementations.
- **Probabilistic `bool`** &mdash; Boolean values that represent
  quantum superposition probabilities. The `qpp/pbool.h` header
  implements a simple `pbool` type that stores the likelihood of
  a value being `true` and provides logical operations that combine
  those probabilities.
- **Bitwise gate macros** &mdash; Convenience wrappers for common
  quantum logic gates such as `H`, `X`, `Y`, `Z`, `CNOT` and `CCX`.
- **Hardware API stubs** &mdash; Placeholder interfaces intended for
  integration with future quantum devices.
- **Hardware detection fallback** &mdash; `qpp::hardware_available()`
  checks for a usable quantum backend and probabilistic types such as
  `qpp::pbool` transparently fall back to a classical random number
  generator with basic glitch detection when none is present.
- **Priority-aware scheduler** &mdash; The `qpp::Scheduler` manages
  `task<*>` functions, supports pause/resume semantics and allows
  removing or reprioritizing tasks.
- **`cstruct` and `cclass`** &mdash; Classical-only variants used for
  hybrid modeling alongside quantum structures.
- **`qregister` and `cregister`** &mdash; Explicit register allocation or
  compiler inference when `register` is left as `auto`.
- **`task<\*>` annotations** &mdash; Functions targeted to the QPU, CPU or
  determined automatically.
- **`__qasm` blocks** &mdash; Inject raw QASM instructions within the
  source code.
- **`#explain` directive** &mdash; Request runtime explanations for upcoming
  instructions.
- **QIR-annotated LLVM IR** &mdash; Compiler output includes Quantum IR with
  probability metadata for hardware backends.
- **`features.yaml` roadmap** &mdash; A YAML file enumerates remaining tasks
  required for a production-ready implementation.

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

### Example

After building, you can compile the demonstration program in
`examples/pbool_demo.cpp`:

```sh
g++ -Iinclude examples/pbool_demo.cpp -o pbool_demo
./pbool_demo
```

### Parsing Example

The repository includes a small prototype parser that emits a JSON
representation of Q++ constructs. Run it on the provided sample file:

```sh
python3 contrib/qpp_parse_ir.py examples/qpp_parse_example.qpp
```

Additional design notes can be found in the files under
`docs/architecture`, covering the frontend, runtime and hardware API.

Refer to the upstream GCC README for additional information about the
compiler and licensing.

