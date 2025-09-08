#ifndef QPP_SIM_FFT_HPP
#define QPP_SIM_FFT_HPP

#include <complex>
#include <vector>

namespace qpp::sim {

/// Thin wrapper around FFT libraries. Falls back to a no-op implementation
/// when neither FFTW nor kissfft is available.
class FFT {
public:
    std::vector<std::complex<double>>
    forward(const std::vector<std::complex<double>>& in) const {
#ifdef QPP_FFTW_AVAILABLE
        // Implementation using FFTW would go here
        // Placeholder returning input
        return in;
#elif defined(QPP_KISSFFT_AVAILABLE)
        // Implementation using kissfft would go here
        // Placeholder returning input
        return in;
#else
        // No FFT library available
        return in;
#endif
    }

    std::vector<std::complex<double>>
    inverse(const std::vector<std::complex<double>>& in) const {
#ifdef QPP_FFTW_AVAILABLE
        // Implementation using FFTW would go here
        return in;
#elif defined(QPP_KISSFFT_AVAILABLE)
        // Implementation using kissfft would go here
        return in;
#else
        return in;
#endif
    }
};

} // namespace qpp::sim

#endif // QPP_SIM_FFT_HPP
