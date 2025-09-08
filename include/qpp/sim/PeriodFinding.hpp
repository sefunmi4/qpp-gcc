#ifndef QPP_SIM_PERIODFINDING_HPP
#define QPP_SIM_PERIODFINDING_HPP

#include <cstddef>
#include <complex>
#include <vector>
#include <cmath>

#include "FFT.hpp"

namespace qpp::sim {

/// Estimate the period of f(x) = a^x mod N using an FFT approach.
inline std::size_t period_finding(unsigned a, unsigned N, std::size_t samples) {
    std::vector<double> sequence(samples);
    unsigned value = 1;
    for (std::size_t i = 0; i < samples; ++i) {
        sequence[i] = static_cast<double>(value);
        value = static_cast<unsigned long long>(value) * a % N;
    }

    FFT fft;
    auto spectrum = fft.rfft(sequence);

    double max_mag = 0.0;
    std::size_t max_index = 0;
    for (std::size_t i = 1; i < spectrum.size(); ++i) {
        double mag = std::norm(spectrum[i]);
        if (mag > max_mag) {
            max_mag = mag;
            max_index = i;
        }
    }

    if (max_index == 0)
        return 0;
    return samples / max_index;
}

} // namespace qpp::sim

#endif // QPP_SIM_PERIODFINDING_HPP
