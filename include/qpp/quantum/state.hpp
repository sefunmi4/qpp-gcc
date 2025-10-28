#pragma once

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstddef>
#include <vector>

namespace qpp::quantum {

struct qstate {
    std::vector<std::complex<double>> amplitude{};
    mutable std::vector<double> probabilities{};
    mutable bool probabilities_dirty = true;
};

inline void assign_uniform(qstate& state, std::size_t count) {
    state.amplitude.assign(count, {0.0, 0.0});
    state.probabilities.assign(count, 0.0);
    if (count == 0) {
        state.probabilities_dirty = false;
        return;
    }
    const double amplitude = 1.0 / std::sqrt(static_cast<double>(count));
    for (auto& amp : state.amplitude)
        amp = {amplitude, 0.0};
    std::fill(state.probabilities.begin(), state.probabilities.end(),
              1.0 / static_cast<double>(count));
    state.probabilities_dirty = false;
}

inline double amplitude_norm(const qstate& state) {
    double total = 0.0;
    for (const auto& amp : state.amplitude)
        total += std::norm(amp);
    return total;
}

inline void normalize_amplitudes(qstate& state) {
    const double total = amplitude_norm(state);
    if (state.amplitude.empty()) {
        state.probabilities.clear();
        state.probabilities_dirty = false;
        return;
    }
    if (total == 0.0) {
        assign_uniform(state, state.amplitude.size());
        return;
    }
    const double inv = 1.0 / std::sqrt(total);
    for (auto& amp : state.amplitude)
        amp *= inv;
    state.probabilities_dirty = true;
}

inline void ensure_probabilities(qstate& state) {
    if (!state.probabilities_dirty)
        return;
    state.probabilities.resize(state.amplitude.size(), 0.0);
    const double total = amplitude_norm(state);
    if (total == 0.0) {
        if (!state.amplitude.empty()) {
            const double uniform = 1.0 / static_cast<double>(state.amplitude.size());
            std::fill(state.probabilities.begin(), state.probabilities.end(), uniform);
        }
        state.probabilities_dirty = false;
        return;
    }
    const double inv = 1.0 / total;
    for (std::size_t i = 0; i < state.amplitude.size(); ++i)
        state.probabilities[i] = std::norm(state.amplitude[i]) * inv;
    state.probabilities_dirty = false;
}

inline void resize_preserving_norm(qstate& state, std::size_t count) {
    const std::size_t old = state.amplitude.size();
    if (old == count) {
        if (count == 0)
            state.probabilities_dirty = false;
        return;
    }
    if (old == 0) {
        assign_uniform(state, count);
        return;
    }
    state.amplitude.resize(count, {0.0, 0.0});
    state.probabilities_dirty = true;
    if (count == 0) {
        state.probabilities.clear();
        state.probabilities_dirty = false;
        return;
    }
    normalize_amplitudes(state);
}

inline void reset_state(qstate& state, std::size_t count) {
    assign_uniform(state, count);
}

inline void mark_dirty(qstate& state) {
    state.probabilities_dirty = true;
}

inline void collapse(qstate& state, std::size_t index) {
    if (index >= state.amplitude.size())
        return;
    for (std::size_t i = 0; i < state.amplitude.size(); ++i)
        state.amplitude[i] = (i == index) ? std::complex<double>{1.0, 0.0}
                                          : std::complex<double>{0.0, 0.0};
    state.probabilities.assign(state.amplitude.size(), 0.0);
    if (index < state.probabilities.size())
        state.probabilities[index] = 1.0;
    state.probabilities_dirty = false;
}

inline double probability_of(qstate& state, std::size_t index) {
    ensure_probabilities(state);
    if (index >= state.probabilities.size())
        return 0.0;
    return state.probabilities[index];
}

} // namespace qpp::quantum

