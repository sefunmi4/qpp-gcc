# Quantum-aware 1D Dynamic Programming

The 1D dynamic programming demonstrations now expose quantum-inspired data
structures so that every algorithm both consumes and produces quantum-aware
containers.  Each routine accepts `qpp::qdp::quantum_sequence` or
`qpp::qdp::quantum_string` inputs and returns `qpp::qdp::quantum_scalar`
results, ensuring that the example showcases how classical DP workflows can be
embedded in quantum-friendly abstractions.

The accompanying example application
(`examples/data-structures-and-algorithms/dynamic-programming-1d/dynamic_programming_1d.qpp`)
instantiates the new quantum containers, exercises the algorithms, and prints
the resulting amplitudes.  Automated regression coverage lives in
`tests/test_dynamic_programming_quantum.cpp` where representative algorithms are
invoked and validated against expected classical outcomes while still running
through the quantum wrappers.
