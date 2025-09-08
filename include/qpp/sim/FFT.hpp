#ifndef QPP_SIM_FFT_HPP
#define QPP_SIM_FFT_HPP

#include <complex>
#include <vector>

namespace qpp::sim {

#ifdef QPP_SINGLE_PRECISION
using real_t = float;
#else
using real_t = double;
#endif
using complex_t = std::complex<real_t>;

/// Thin wrapper around FFT libraries. Falls back to a no-op implementation
/// when no supported FFT backend is available.
class FFT {
public:
    std::vector<complex_t>
    forward(const std::vector<complex_t>& in) const {
#if defined(QPP_USE_FFTW)
        // Implementation using FFTW would go here
        return in;
#elif defined(QPP_USE_ONEMKL)
        // Implementation using oneMKL would go here
        return in;
#elif defined(QPP_USE_CUFFT)
        // Implementation using cuFFT would go here
        return in;
#else
        // No FFT library available
        return in;
#endif
    }

    /// Compute real-to-complex FFT of a real input sequence.
    std::vector<complex_t> rfft(const std::vector<real_t>& in) const {
        std::vector<complex_t> complex_in(in.begin(), in.end());
        return forward(complex_in);
    }

    std::vector<complex_t>
    inverse(const std::vector<complex_t>& in) const {
#if defined(QPP_USE_FFTW)
        // Implementation using FFTW would go here
        return in;
#elif defined(QPP_USE_ONEMKL)
        // Implementation using oneMKL would go here
        return in;
#elif defined(QPP_USE_CUFFT)
        // Implementation using cuFFT would go here
        return in;
#else
        return in;
#endif
    }
};

} // namespace qpp::sim

#endif // QPP_SIM_FFT_HPP
