# Quantum Linked List Toolkit

This example upgrades the classical linked list exercises to operate on the
project's quantum data structures.  Each node stores a [`qint`](../../../include/qpp/qint)
payload and traversal utilities return `std::qvector` containers so that
algorithms can remain quantum-aware while still collapsing values for
presentation.

The executable example `linked_list_problems.qpp` demonstrates how the helper
functions expose both quantum and classical views of the structure:

- `to_vector` returns the quantum payloads after ensuring they are measured and
  entanglement-safe.
- `to_classical_vector` provides deterministic classical integers for logging.
- Cycle-aware traversal uses `std::entangled_set` to prevent infinite loops
  when sampling lists with shared structure.

Running the example prints both the high-level description as well as the
collapsed measurement results, confirming that the algorithms continue to
produce the expected orderings using the quantum node representation.
