#ifndef QPP_SIM_STATEVECTOR_HPP
#define QPP_SIM_STATEVECTOR_HPP

#include <complex>
#include <vector>
#include <cmath>
#include <cstddef>

namespace qpp {
namespace sim {

/// Simple state vector representation using big-endian qubit ordering.
struct StateVector {
    /// Amplitude data in computational basis order
    std::vector<std::complex<double>> data;
    /// Number of qubits represented by the state
    std::size_t num_qubits = 0;

    /// Allocate space for given number of qubits and reset to |0...0>
    void allocate(std::size_t nq) {
        num_qubits = nq;
        data.assign(std::size_t{1} << nq, {0.0, 0.0});
        if (!data.empty())
            data[0] = {1.0, 0.0};
    }

    /// Normalize amplitudes so that total probability equals one
    void normalize() {
        double norm = 0.0;
        for (const auto &amp : data)
            norm += std::norm(amp);
        if (norm == 0.0)
            return;
        double inv = 1.0 / std::sqrt(norm);
        for (auto &amp : data)
            amp *= inv;
    }

    /// Probability of a specific basis state index
    double probability(std::size_t index) const {
        return std::norm(data[index]);
    }

    /// Probabilities for all basis states
    std::vector<double> probabilities() const {
        std::vector<double> probs(data.size());
        for (std::size_t i = 0; i < data.size(); ++i)
            probs[i] = std::norm(data[i]);
        return probs;
    }

    /// Measurement probability distribution for selected qubits.
    /// Qubits are specified using big-endian order (q=0 is highest bit).
    /// Returns a vector of size 2^k where k = qubits.size(), with
    /// probabilities ordered in big-endian bit order of the measured qubits.
    std::vector<double> measure_probs(const std::vector<int> &qubits) const {
        if (qubits.empty())
            return probabilities();
        std::size_t k = qubits.size();
        std::vector<double> probs(std::size_t{1} << k, 0.0);
        for (std::size_t i = 0; i < data.size(); ++i) {
            double p = std::norm(data[i]);
            if (p == 0.0)
                continue;
            std::size_t outcome = 0;
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
