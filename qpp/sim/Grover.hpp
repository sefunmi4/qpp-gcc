#ifndef QPP_SIM_GROVER_HPP
#define QPP_SIM_GROVER_HPP

#include <cstddef>
#include <complex>
#include <functional>

#include "StateVector.hpp"

namespace qpp {
namespace sim {

/// Perform amplitude amplification (Grover's algorithm) on a state vector.
/// The oracle marks basis states for which the provided predicate returns true.
inline void amplitude_amplification(StateVector &psi,
                                   std::function<bool(std::size_t)> oracle,
                                   int iters) {
    std::size_t N = psi.data.size();
    if (N == 0 || iters <= 0)
        return;
    for (int k = 0; k < iters; ++k) {
        // Oracle phase flip
        for (std::size_t i = 0; i < N; ++i) {
            if (oracle(i))
                psi.data[i] = -psi.data[i];
        }
        // Diffusion operator (reflection about uniform superposition)
        std::complex<double> avg{0.0, 0.0};
        for (const auto &amp : psi.data)
            avg += amp;
        avg /= static_cast<double>(N);
        for (auto &amp : psi.data)
            amp = 2.0 * avg - amp;
    }
    psi.normalize();
}

} // namespace sim
} // namespace qpp

#endif // QPP_SIM_GROVER_HPP
