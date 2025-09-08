#ifndef QPP_SIM_GATES_HPP
#define QPP_SIM_GATES_HPP

#include <algorithm>
#include <array>
#include <complex>
#include <cstddef>
#include <cmath>
#if defined(QPP_ENABLE_OPENMP) || defined(QPP_ENABLE_GPU)
#include <omp.h>
#endif

#include "StateVector.hpp"

namespace qpp {
namespace sim {

using GateMatrix = std::array<complex_t, 4>; // row-major 2x2

inline const GateMatrix X{{ complex_t{0.0, 0.0}, complex_t{1.0, 0.0},
                            complex_t{1.0, 0.0}, complex_t{0.0, 0.0} }};
inline const GateMatrix Z{{ complex_t{1.0, 0.0}, complex_t{0.0, 0.0},
                            complex_t{0.0, 0.0}, complex_t{-1.0, 0.0} }};
constexpr real_t INV_SQRT2 =
    static_cast<real_t>(0.70710678118654752440084436210485L);
inline const GateMatrix H{{ complex_t{INV_SQRT2, 0.0}, complex_t{INV_SQRT2, 0.0},
                            complex_t{INV_SQRT2, 0.0}, complex_t{-INV_SQRT2, 0.0} }};

/// Apply arbitrary single-qubit gate matrix to qubit q.
inline void apply_gate(StateVector &psi, const GateMatrix &U, std::size_t q) {
    std::size_t bit = psi.num_qubits - 1 - q;
    std::size_t stride = std::size_t{1} << bit;
    std::size_t period = stride << 1;
    std::size_t total = psi.data.size();
#if defined(QPP_ENABLE_GPU)
#pragma omp target teams distribute parallel for collapse(2) schedule(static)
#elif defined(QPP_ENABLE_OPENMP)
#pragma omp parallel for collapse(2) schedule(static)
#endif
    for (std::size_t i = 0; i < total; i += period) {
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
#if defined(QPP_ENABLE_GPU)
#pragma omp target teams distribute parallel for
#elif defined(QPP_ENABLE_OPENMP)
#pragma omp parallel for
#endif
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
#if defined(QPP_ENABLE_GPU)
#pragma omp target teams distribute parallel for
#elif defined(QPP_ENABLE_OPENMP)
#pragma omp parallel for
#endif
    for (std::size_t i = 0; i < psi.data.size(); ++i) {
        if (((i >> b1) & 1u) && ((i >> b2) & 1u) && !((i >> bt) & 1u)) {
            std::size_t j = i | (std::size_t{1} << bt);
            std::swap(psi.data[i], psi.data[j]);
        }
    }
}

/// Apply rotation around Z-axis by given angle to qubit q
inline void apply_rz(StateVector &psi, real_t angle, std::size_t q) {
    std::size_t bit = psi.num_qubits - 1 - q;
    std::size_t stride = std::size_t{1} << bit;
    std::size_t period = stride << 1;
    complex_t phase0 = std::exp(complex_t{0, -angle / 2});
    complex_t phase1 = std::exp(complex_t{0, angle / 2});
#if defined(QPP_ENABLE_GPU)
#pragma omp target teams distribute parallel for collapse(2) schedule(static)
#elif defined(QPP_ENABLE_OPENMP)
#pragma omp parallel for collapse(2) schedule(static)
#endif
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
