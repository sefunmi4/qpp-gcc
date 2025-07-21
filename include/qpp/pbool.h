#ifndef QPP_PBOOL_H
#define QPP_PBOOL_H

#include <algorithm>

namespace qpp {

/// Probabilistic boolean value representing quantum superposition.
/// Stores the probability of the value being `true`.
class pbool {
    double prob_true;
public:
    /// Construct with a probability in [0,1]. Values outside the range are clamped.
    constexpr pbool(double p = 0.0) : prob_true(std::clamp(p, 0.0, 1.0)) {}

    /// Return the probability that the value is true.
    constexpr double probability() const { return prob_true; }

    /// Convert to classical bool using 0.5 as threshold.
    explicit constexpr operator bool() const { return prob_true >= 0.5; }

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

} // namespace qpp

#endif // QPP_PBOOL_H
