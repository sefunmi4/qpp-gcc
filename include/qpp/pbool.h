#ifndef QPP_PBOOL_H
#define QPP_PBOOL_H

#include <algorithm>
#include <atomic>
#include <random>
#include <stdexcept>
#include "hardware.hpp"

namespace qpp {

/// Probabilistic boolean value representing quantum superposition.
/// Stores the probability of the value being `true`.
class pbool {
    double prob_true;
    static std::mt19937& rng() {
        static thread_local std::mt19937 gen(std::random_device{}());
        return gen;
    }
    static std::atomic<int> glitch_count;
public:
    /// Construct with a probability in [0,1]. Values outside the range are clamped.
    constexpr pbool(double p = 0.0) : prob_true(std::clamp(p, 0.0, 1.0)) {}

    /// Return the probability that the value is true.
    constexpr double probability() const { return prob_true; }

    /// Convert to classical bool. When quantum hardware is available this
    /// performs a probabilistic sample; otherwise it falls back to a
    /// deterministic threshold.
    explicit operator bool() const {
        if (hardware_available())
            return sample();
        return prob_true >= 0.5;
    }

    /// Draw a random sample according to the stored probability. A "glitch"
    /// is detected when the generated number is extremely close to 0 or 1.
    /// In that case the function guesses the outcome deterministically. After
    /// several successive glitches a runtime_error is thrown to signal that
    /// the application should shut down to avoid undefined behavior.
    bool sample() const {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        double r = dist(rng());
        if (r < 1e-6 || r > 1.0 - 1e-6) {
            if (++glitch_count > 3)
                throw std::runtime_error(
                    "Unrecoverable RNG glitch detected, shutting down");
            return prob_true >= 0.5;
        }
        glitch_count = 0;
        return r < prob_true;
    }

    /// Logical AND on probabilities.
    friend constexpr pbool operator&&(const pbool& a, const pbool& b) {
        return pbool(a.prob_true * b.prob_true);
    }

    /// Logical OR on probabilities.
    friend constexpr pbool operator||(const pbool& a, const pbool& b) {
        return pbool(a.prob_true + b.prob_true - a.prob_true * b.prob_true);
    }

    /// Logical NOT on probabilities.
    friend constexpr pbool operator!(const pbool& a) {
        return pbool(1.0 - a.prob_true);
    }
};

// Definition of the static glitch counter
inline std::atomic<int> pbool::glitch_count{0};

} // namespace qpp

#endif // QPP_PBOOL_H
