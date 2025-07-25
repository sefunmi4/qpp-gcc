#ifndef QPP_QSTRUCT_HPP
#define QPP_QSTRUCT_HPP

#include <complex>
#include <vector>
#include <cstddef>
#include <cmath>
#include "pbool.h"

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

    /// Apply Pauli-Y gate to a qubit
    void apply_y(std::size_t qubit) {
        const std::size_t stride = std::size_t{1} << qubit;
        const std::size_t period = stride << 1;
        const std::complex<double> I(0.0, 1.0);
        for (std::size_t i = 0; i < state.amplitude.size(); i += period) {
            for (std::size_t j = 0; j < stride; ++j) {
                auto a0 = state.amplitude[i + j];
                auto a1 = state.amplitude[i + j + stride];
                state.amplitude[i + j] = -I * a1;
                state.amplitude[i + j + stride] = I * a0;
            }
        }
    }

    /// Apply Pauli-Z gate to a qubit
    void apply_z(std::size_t qubit) {
        const std::size_t stride = std::size_t{1} << qubit;
        const std::size_t period = stride << 1;
        for (std::size_t i = 0; i < state.amplitude.size(); i += period) {
            for (std::size_t j = 0; j < stride; ++j) {
                state.amplitude[i + j + stride] *= -1.0;
            }
        }
    }

    /// Apply controlled-X gate
    void apply_cx(std::size_t control, std::size_t target) {
        if (control == target)
            return;
        for (std::size_t i = 0; i < state.amplitude.size(); ++i) {
            if (((i >> control) & 1) && !((i >> target) & 1)) {
                std::size_t j = i | (std::size_t{1} << target);
                std::swap(state.amplitude[i], state.amplitude[j]);
            }
        }
    }

    /// Apply Toffoli (CCX) gate
    void apply_ccx(std::size_t control1, std::size_t control2, std::size_t target) {
        if (control1 == target || control2 == target)
            return;
        for (std::size_t i = 0; i < state.amplitude.size(); ++i) {
            if (((i >> control1) & 1) && ((i >> control2) & 1) && !((i >> target) & 1)) {
                std::size_t j = i | (std::size_t{1} << target);
                std::swap(state.amplitude[i], state.amplitude[j]);
            }
        }
    }

    /// Return probability that the given qubit is measured as 1
    pbool measure(size_t qubit) const {
        double p1 = 0.0;
        for (std::size_t i = 0; i < state.amplitude.size(); ++i)
            if ((i >> qubit) & 1)
                p1 += std::norm(state.amplitude[i]);
        return pbool(p1);
    }

private:
    qstruct state;
};

} // namespace qpp

#endif // QPP_QSTRUCT_HPP
