#ifndef QPP_QSTRUCT_HPP
#define QPP_QSTRUCT_HPP

#include <complex>
#include <vector>
#include <cstddef>
#include <cmath>

namespace qpp {

/// Basic quantum state container
struct qstruct {
    /// Amplitude vector
    std::vector<std::complex<double>> amplitude;

    /// Construct empty state with optional number of qubits
    explicit qstruct(std::size_t num_qubits = 0) {
        reset(num_qubits);
    }

    /// Reset with new number of qubits
    void reset(std::size_t num_qubits) {
        amplitude.assign(std::size_t{1} << num_qubits, {0.0, 0.0});
        if (!amplitude.empty())
            amplitude[0] = {1.0, 0.0}; // |0...0>
    }

    /// Return number of qubits in the state
    std::size_t qubits() const {
        std::size_t sz = amplitude.size();
        std::size_t q = 0;
        while (sz > 1) {
            sz >>= 1;
            ++q;
        }
        return q;
    }
};

/// Simple quantum register manipulation class
class qclass {
public:
    explicit qclass(std::size_t num_qubits = 0) : state(num_qubits) {}

    /// Access underlying state
    qstruct& data() { return state; }
    const qstruct& data() const { return state; }

    /// Apply Pauli-X gate to a qubit
    void apply_x(std::size_t qubit) {
        const std::size_t stride = std::size_t{1} << qubit;
        const std::size_t period = stride << 1;
        for (std::size_t i = 0; i < state.amplitude.size(); i += period)
            for (std::size_t j = 0; j < stride; ++j)
                std::swap(state.amplitude[i + j], state.amplitude[i + j + stride]);
    }

    /// Apply Hadamard gate to a qubit (not normalized)
    void apply_h(std::size_t qubit) {
        const std::size_t stride = std::size_t{1} << qubit;
        const std::size_t period = stride << 1;
        const double inv_sqrt2 = 1.0 / std::sqrt(2.0);
        for (std::size_t i = 0; i < state.amplitude.size(); i += period) {
            for (std::size_t j = 0; j < stride; ++j) {
                auto a0 = state.amplitude[i + j];
                auto a1 = state.amplitude[i + j + stride];
                state.amplitude[i + j] = (a0 + a1) * inv_sqrt2;
                state.amplitude[i + j + stride] = (a0 - a1) * inv_sqrt2;
            }
        }
    }

private:
    qstruct state;
};

} // namespace qpp

#endif // QPP_QSTRUCT_HPP
