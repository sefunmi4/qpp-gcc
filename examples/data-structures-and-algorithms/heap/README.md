# Quantum Heap Median Finder

The median finder example now relies on the reusable
[`qpp::quantum::QuantumHeap`](../../../qpp/quantum/quantum_heap.hpp) implementation.
Both halves of the streaming median structure share the same backend configuration so
that comparisons, rebalancing, and element transfers are all routed through the quantum
sampling pipeline.

To exercise the quantum heap directly, build and run the `median_finder.qpp` sample. In
addition to the running median demonstration, it now prints the results of a standalone
max/min heap transfer driven by the quantum backend so you can confirm that the
probabilistic ordering is wired up correctly.

```sh
qpp++ examples/data-structures-and-algorithms/heap/median_finder.qpp
```

The program seeds the backend with a deterministic value and uses 128 sampling shots so
results are reproducible while still showcasing the quantum-aware APIs.
