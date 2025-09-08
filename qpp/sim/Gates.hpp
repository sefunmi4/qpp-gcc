#ifndef QPP_SIM_GATES_HPP
#define QPP_SIM_GATES_HPP

#include <algorithm>
#include <array>
#include <complex>
#include <cstddef>
#include <cmath>

#include "StateVector.hpp"

namespace qpp {
namespace sim {

using GateMatrix = std::array<std::complex<double>, 4>; // row-major 2x2

inline const GateMatrix X{{{0.0, 0.0}, {1.0, 0.0}, {1.0, 0.0}, {0.0, 0.0}}};
inline const GateMatrix Z{{{1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {-1.0, 0.0}}};
inline const double INV_SQRT2 = 0.70710678118654752440084436210485;
inline const GateMatrix H{{{INV_SQRT2, 0.0}, {INV_SQRT2, 0.0}, {INV_SQRT2, 0.0}, {-INV_SQRT2, 0.0}}};

/// Apply arbitrary single-qubit gate matrix to qubit q.
inline void apply_gate(StateVector &psi, const GateMatrix &U, std::size_t q) {
    std::size_t bit = psi.num_qubits - 1 - q;
    std::size_t stride = std::size_t{1} << bit;
    std::size_t period = stride << 1;
    for (std::size_t i = 0; i < psi.data.size(); i += period) {
        for (std::size_t j = 0; j < stride; ++j) {
            auto a0 = psi.data[i + j];
            auto a1 = psi.data[i + j + stride];
            psi.data[i + j] = U[0] * a0 + U[1] * a1;
            psi.data[i + j + stride] = U[2] * a0 + U[3] * a1;
        }
    }
}

/// Apply controlled-NOT with big-endian qubit indexing
inline void apply_cx(StateVector &psi, std::size_t control, std::size_t target) {
    if (control == target)
        return;
    std::size_t n = psi.num_qubits;
    std::size_t cbit = n - 1 - control;
    std::size_t tbit = n - 1 - target;
    for (std::size_t i = 0; i < psi.data.size(); ++i) {
        if (((i >> cbit) & 1u) && !((i >> tbit) & 1u)) {
            std::size_t j = i | (std::size_t{1} << tbit);
            std::swap(psi.data[i], psi.data[j]);
        }
    }
}

/// Apply Toffoli gate with two controls
inline void apply_ccx(StateVector &psi, std::size_t c1, std::size_t c2, std::size_t target) {
    if (c1 == target || c2 == target)
        return;
    std::size_t n = psi.num_qubits;
    std::size_t b1 = n - 1 - c1;
    std::size_t b2 = n - 1 - c2;
    std::size_t bt = n - 1 - target;
    for (std::size_t i = 0; i < psi.data.size(); ++i) {
        if (((i >> b1) & 1u) && ((i >> b2) & 1u) && !((i >> bt) & 1u)) {
            std::size_t j = i | (std::size_t{1} << bt);
            std::swap(psi.data[i], psi.data[j]);
        }
    }
}

/// Apply rotation around Z-axis by given angle to qubit q
inline void apply_rz(StateVector &psi, double angle, std::size_t q) {
    std::size_t bit = psi.num_qubits - 1 - q;
    std::size_t stride = std::size_t{1} << bit;
    std::size_t period = stride << 1;
    std::complex<double> phase0 = std::exp(std::complex<double>(0, -angle / 2));
    std::complex<double> phase1 = std::exp(std::complex<double>(0, angle / 2));
    for (std::size_t i = 0; i < psi.data.size(); i += period) {
        for (std::size_t j = 0; j < stride; ++j) {
            psi.data[i + j] *= phase0;
            psi.data[i + j + stride] *= phase1;
        }
    }
}

} // namespace sim
} // namespace qpp

#endif // QPP_SIM_GATES_HPP
