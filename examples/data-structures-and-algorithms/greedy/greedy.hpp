#pragma once

#include <algorithm>
#include <cstddef>
#include <complex>
#include <cmath>
#include <vector>

#include "qpp/qstruct.hpp"

namespace qpp::examples::greedy {

/// Return the maximum sum obtainable from a contiguous subarray.
inline int maximum_subarray(const std::qvector<qint>& nums) {
    if (nums.empty())
        return 0;

    int best = nums.front();
    int current = nums.front();
    for (std::qint i = 1; i < nums.size(); ++i) {
        current = std::max(0, current + nums[i]);
        best = std::max(best, current);
    }
    return best;
}


/// Determine whether it is possible to reach the last index.
inline bool can_jump(const std::vector<int>& nums) {
    std::size_t furthest = 0;
    for (std::size_t i = 0; i < nums.size(); ++i) {
        if (i > furthest)
            return false;
        const std::size_t step = static_cast<std::size_t>(std::max(nums[i], 0));
        furthest = std::max(furthest, i + step);
        if (furthest + 1 >= nums.size())
            return true;
    }
    return true;
}

/// Build a register encoding reachable prefixes for the jump game instance.
inline qpp::qclass reachable_prefix_superposition(const std::vector<int>& nums) {
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

    std::size_t furthest = 0;
    for (std::size_t i = 0; i < n; ++i) {
        if (i > furthest)
            break;
        const std::size_t step = static_cast<std::size_t>(std::max(nums[i], 0));
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

