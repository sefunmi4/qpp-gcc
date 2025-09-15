# Q++ Syntax Example

## Overview
`qpp_parse_example.qpp` is a compact showcase of Q++ language constructs used by the custom GCC frontend. It exercises probabilistic storage types, hybrid task annotations, intrinsic QASM blocks, and gate-overloaded operators.

## Language Features Highlighted
- `qstruct` definitions that combine quantum (`qbit`) and classical fields in a single aggregate.
- Automatic allocation of `qregister`/`cregister` storage for quantum and classical data.
- Overloaded bitwise operators (`^`, `&`) mapping to entangling gates when applied to quantum operands.
- Inline `__qasm { ... }` blocks that embed raw OpenQASM instructions for direct backend emission.
- `task<QPU>` annotated functions that mark kernels intended for execution on hardware.

## Usage Notes
This file is intended for parser and tooling validation. It is not part of the default CMake build, and compiling it requires the extended Q++ toolchain rather than a standard C++ compiler. Treat it as a reference while writing or testing new language features.
