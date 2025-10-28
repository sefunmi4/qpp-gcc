#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <complex>
#include <cmath>
#include <vector>

#include "qpp/qint"
#include "qpp/qstruct.hpp"
#include "qpp/qvector"

namespace qpp::examples::greedy {

namespace detail {

inline std::array<int, 4> sample_axes(const qint& value) {
    return {value.x_step()(), value.y_step()(), value.z_step()(), value.t_step()()};
}

inline int collapse_axes_to_scalar(const std::array<int, 4>& axes) {
    const int sign = axes[0] >= 0 ? 1 : -1;
    int magnitude = 0;
    for (std::size_t bit = 1; bit < axes.size(); ++bit) {
        if (axes[bit] > 0)
            magnitude |= 1 << (bit - 1);
    }
    return sign * magnitude;
}

inline int collapse_to_scalar(const qint& value) {
    return collapse_axes_to_scalar(sample_axes(value));
}

} // namespace detail

/// Return the maximum sum obtainable from a contiguous subarray.
inline int maximum_subarray(const qpp::qvector<qint>& nums) {
    if (nums.empty())
        return 0;

    const int first_value = detail::collapse_to_scalar(nums.front());
    int best = first_value;
    int current = first_value;
    for (std::size_t i = 1; i < nums.size(); ++i) {
        const int value = detail::collapse_to_scalar(nums[i]);
        current = std::max(0, current + value);
        best = std::max(best, current);
    }
    return best;
}


/// Determine whether it is possible to reach the last index.
inline bool can_jump(const qpp::qvector<qint>& nums) {
    const auto collapse_step = [](const qint& value) {
        return detail::collapse_to_scalar(value);
    };

    std::size_t furthest = 0;
    for (std::size_t i = 0; i < nums.size(); ++i) {
        if (i > furthest)
            return false;
        const int collapsed = collapse_step(nums[i]);
        const std::size_t step = static_cast<std::size_t>(std::max(collapsed, 0));
        furthest = std::max(furthest, i + step);
        if (furthest + 1 >= nums.size())
            return true;
    }
    return true;
}

/// Build a register encoding reachable prefixes for the jump game instance.
inline qpp::qclass reachable_prefix_superposition(const qpp::qvector<qint>& nums) {
    const std::size_t n = nums.size();
    std::size_t qubits = 0;
    while ((std::size_t{1} << qubits) < std::max<std::size_t>(n, 1))
        ++qubits;

    qpp::qclass reg(qubits);
    if (qubits == 0)
        return reg;

    for (std::size_t q = 0; q < qubits; ++q)
        reg.apply_h(q);

    auto& amplitude = reg.data().amplitude;
    for (std::size_t basis = n; basis < amplitude.size(); ++basis)
        amplitude[basis] = {0.0, 0.0};

    if (n == 0)
        return reg;

    const auto collapse_step = [](const qint& value) {
        return detail::collapse_to_scalar(value);
    };

    std::size_t furthest = 0;
    for (std::size_t i = 0; i < n; ++i) {
        if (i > furthest)
            break;
        const int collapsed = collapse_step(nums[i]);
        const std::size_t step = static_cast<std::size_t>(std::max(collapsed, 0));
        const std::size_t reach = i + step;
        if (reach > furthest)
            furthest = std::min<std::size_t>(reach, n - 1);
    }

    for (std::size_t basis = furthest + 1; basis < n; ++basis)
        amplitude[basis] = {0.0, 0.0};

    double norm = 0.0;
    for (const auto& amp : amplitude)
        norm += std::norm(amp);

    if (norm > 0.0) {
        const double inv_norm = 1.0 / std::sqrt(norm);
        for (auto& amp : amplitude)
            amp *= inv_norm;
    }

    return reg;
}

} // namespace qpp::examples::greedy

