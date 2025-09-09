#ifndef QPP_SIM_STATEVECTOR_HPP
#define QPP_SIM_STATEVECTOR_HPP

#include <complex>
#include <vector>
#include <cmath>
#include <cstddef>
#if defined(QPP_ENABLE_OPENMP) || defined(QPP_ENABLE_GPU)
#include <omp.h>
#endif

namespace qpp {
namespace sim {

#ifdef QPP_SINGLE_PRECISION
using real_t = float;
#else
using real_t = double;
#endif
using complex_t = std::complex<real_t>;

/// Simple state vector representation using big-endian qubit ordering.
struct StateVector {
    /// Amplitude data in computational basis order
    std::vector<complex_t> data;
    /// Number of qubits represented by the state
    std::size_t num_qubits = 0;

    /// Allocate space for given number of qubits and reset to |0...0>
    void allocate(std::size_t nq) {
        num_qubits = nq;
        data.assign(std::size_t{1} << nq, complex_t{0.0, 0.0});
        if (!data.empty())
            data[0] = complex_t{1.0, 0.0};
    }

    /// Normalize amplitudes so that total probability equals one
    void normalize() {
        real_t norm = 0.0;
#if defined(QPP_ENABLE_GPU)
#pragma omp target teams distribute parallel for reduction(+:norm)
#elif defined(QPP_ENABLE_OPENMP)
#pragma omp parallel for reduction(+:norm)
#endif
        for (std::size_t i = 0; i < data.size(); ++i)
            norm += std::norm(data[i]);
        if (norm == 0.0)
            return;
        real_t inv = real_t{1.0} / std::sqrt(norm);
#if defined(QPP_ENABLE_GPU)
#pragma omp target teams distribute parallel for
#elif defined(QPP_ENABLE_OPENMP)
#pragma omp parallel for
#endif
        for (std::size_t i = 0; i < data.size(); ++i)
            data[i] *= inv;
    }

    /// Probability of a specific basis state index
    real_t probability(std::size_t index) const {
        return std::norm(data[index]);
    }

    /// Probabilities for all basis states
    std::vector<real_t> probabilities() const {
        std::vector<real_t> probs(data.size());
#if defined(QPP_ENABLE_GPU)
#pragma omp target teams distribute parallel for
#elif defined(QPP_ENABLE_OPENMP)
#pragma omp parallel for
#endif
        for (std::size_t i = 0; i < data.size(); ++i)
            probs[i] = std::norm(data[i]);
        return probs;
    }

    /// Measurement probability distribution for selected qubits.
    /// Qubits are specified using big-endian order (q=0 is highest bit).
    /// Returns a vector of size 2^k where k = qubits.size(), with
    /// probabilities ordered in big-endian bit order of the measured qubits.
    std::vector<real_t> measure_probs(const std::vector<int> &qubits) const {
        if (qubits.empty())
            return probabilities();
        std::size_t k = qubits.size();
        std::vector<real_t> probs(std::size_t{1} << k, real_t{0});
#if defined(QPP_ENABLE_GPU)
#pragma omp target teams distribute parallel for
#elif defined(QPP_ENABLE_OPENMP)
#pragma omp parallel for
#endif
        for (std::size_t i = 0; i < data.size(); ++i) {
            real_t p = std::norm(data[i]);
            if (p == real_t{0})
                continue;
            std::size_t outcome = 0;
#if defined(QPP_ENABLE_OPENMP)
#pragma omp simd
#endif
            for (std::size_t j = 0; j < k; ++j) {
                int q = qubits[j];
                std::size_t bit = num_qubits - 1 - static_cast<std::size_t>(q);
                std::size_t val = (i >> bit) & 1u;
                outcome = (outcome << 1) | val;
            }
            probs[outcome] += p;
        }
        return probs;
    }
};

} // namespace sim
} // namespace qpp

#endif // QPP_SIM_STATEVECTOR_HPP
