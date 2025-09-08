#ifndef QPP_SIM_COMPACTIFICATION_HPP
#define QPP_SIM_COMPACTIFICATION_HPP

#include <cmath>
#include <complex>

namespace qpp::sim {

// Map a real number to a point on the unit circle S^1
inline std::complex<double> compactifyRtoS1(double x) {
    return {std::cos(x), std::sin(x)};
}

// Recover an angle in [0,2pi) from a point on S^1
inline double decompactifyS1toR(const std::complex<double>& z) {
    constexpr double PI2 = 2.0 * 3.14159265358979323846;
    double angle = std::atan2(z.imag(), z.real());
    if (angle < 0)
        angle += PI2;
    return angle;
}

} // namespace qpp::sim

#endif // QPP_SIM_COMPACTIFICATION_HPP
