#ifndef QPP_SIM_ENCODINGS_HPP
#define QPP_SIM_ENCODINGS_HPP

#include <cmath>

namespace qpp::sim {

/// Linear frequency code
struct LFC {
    double scale;
    bool squareWave;

    explicit LFC(double scale_ = 1.0, bool square_wave = false)
        : scale(scale_), squareWave(square_wave) {}

    /// Compute frequency for a value
    double freq(double value) const { return scale * value; }

    /// Encode value at time t as either sine or square wave sample
    double encode(double value, double t) const {
        constexpr double PI2 = 2.0 * 3.14159265358979323846;
        double phase = PI2 * freq(value) * t;
        double s = std::sin(phase);
        return squareWave ? (s >= 0.0 ? 1.0 : -1.0) : s;
    }

    /// Recover value from frequency
    double decodeValue(double frequency) const { return frequency / scale; }
};

/// Logarithmic frequency code
struct LogFC {
    double scale;
    bool squareWave;

    explicit LogFC(double scale_ = 1.0, bool square_wave = false)
        : scale(scale_), squareWave(square_wave) {}

    /// Compute frequency for a value
    double freq(double value) const {
        return value > 0.0 ? scale * std::log(value) : 0.0;
    }

    /// Encode value at time t as either sine or square wave sample
    double encode(double value, double t) const {
        constexpr double PI2 = 2.0 * 3.14159265358979323846;
        double phase = PI2 * freq(value) * t;
        double s = std::sin(phase);
        return squareWave ? (s >= 0.0 ? 1.0 : -1.0) : s;
    }

    /// Recover value from frequency
    double decodeValue(double frequency) const {
        return std::exp(frequency / scale);
    }
};

} // namespace qpp::sim

#endif // QPP_SIM_ENCODINGS_HPP
