# Frontend Overview

This document outlines how the Q++ compiler frontend interprets quantum-aware constructs.

## `task<>` Declarations

```cpp
task<AUTO> hybrid_logic() {}
task<QPU>  quantum_logic() {}
task<CPU>  classical_logic() {}
```

The template parameter specifies the intended execution target or allows the compiler to infer it via `AUTO`.

## Conditional Expressions

```cpp
if (q[0]) { /* ... */ }
```

Conditions depending on quantum memory mark the guarded scope as probabilistic and influence scheduling decisions.

## Memory Allocation

```cpp
qalloc int qarray[4];
qregister int reg1;
register int r2;
```

Explicit quantum or classical registers can be declared. When the `register` keyword is used alone the compiler chooses the appropriate kind.

## Quantum-Aware Types

```cpp
qstruct Token { qbit state; int index; };
```

Structures containing quantum members exhibit probabilistic behavior and are tracked by the scheduler.

## Boolean and Bitwise Behavior

Boolean variables linked to quantum memory become probabilistic:

```cpp
bool flag = q[0];
int x = a & b;  // may emit Toffoli when operands are quantum
```

Bitwise operators expand to the corresponding quantum gates when required.
The current implementation maps `^` to a controlled-X gate (CNOT) and
`&` to a Toffoli gate when the operands reference quantum memory.  Single
qubit operators such as `|`, `~`, and explicit method calls provide access
to additional gates including `Y` and `Z`.
