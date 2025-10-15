#pragma once

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstddef>
#include <vector>

#include "qpp/pbool.h"
#include "qpp/qint"
#include "qpp/qstruct.hpp"
#include "qpp/qvector"

namespace qpp::examples::binary_search {

/// Collapse the logical axis that encodes the classical integer orientation.
inline int measure_value(const qint& value) {
    const auto stats = value.measure_axis(0U);
    if (stats.collapsed_value)
        return *stats.collapsed_value;
    return value.x_step()();
}

/// Locate the minimum element of a rotated sorted array of qints.
inline int find_min_in_rotated_sorted_array(const std::qvector<qint>& nums) {
    if (nums.empty())
        return 0;

    int left = 0;
    int right = static_cast<int>(nums.size()) - 1;
    int best = measure_value(nums.front());

    while (left <= right) {
        const int left_value = measure_value(nums[static_cast<std::size_t>(left)]);
        const int right_value = measure_value(nums[static_cast<std::size_t>(right)]);

        if (left_value <= right_value) {
            best = std::min(best, left_value);
            break;
        }

        const int mid = left + (right - left) / 2;
        const int mid_value = measure_value(nums[static_cast<std::size_t>(mid)]);
        best = std::min(best, mid_value);

        if (mid_value >= left_value)
            left = mid + 1;
        else
            right = mid;
    }

    return best;
}

/// Search for a target element in a rotated sorted array of qints.
inline int search_in_rotated_sorted_array(const std::qvector<qint>& nums,
                                                  int target) {
    if (nums.empty())
        return -1;

    int left = 0;
    int right = static_cast<int>(nums.size()) - 1;

    while (left <= right) {
        const int mid = left + (right - left) / 2;
        const int mid_value = measure_value(nums[static_cast<std::size_t>(mid)]);

        if (mid_value == target)
            return mid;

        const int left_value = measure_value(nums[static_cast<std::size_t>(left)]);
        const int right_value = measure_value(nums[static_cast<std::size_t>(right)]);

        if (left_value <= mid_value) {
            if (target >= left_value && target < mid_value)
                right = mid - 1;
            else
                left = mid + 1;
        } else {
            if (target > mid_value && target <= right_value)
                left = mid + 1;
            else
                right = mid - 1;
        }
    }

    return -1;
}

/// Probability of sampling the pivot when measuring a uniform rotation index state.
inline qpp::pbool pivot_hit_probability(std::size_t length, std::size_t pivot_index) {
    if (length == 0 || pivot_index >= length)
        return qpp::pbool{0.0};

    const double probability = 1.0 / static_cast<double>(length);
    return qpp::pbool{probability};
}

/// Build a register encoding a uniform superposition over rotation pivot indices.
inline qpp::qclass make_pivot_superposition(std::size_t length) {
    std::size_t qubits = 0;
    while ((std::size_t{1} << qubits) < std::max<std::size_t>(length, 1))
        ++qubits;

    qpp::qclass reg(qubits);
    if (qubits == 0)
        return reg;

    for (std::size_t q = 0; q < qubits; ++q)
        reg.apply_h(q);

    auto& amplitude = reg.data().amplitude;
    for (std::size_t basis = length; basis < amplitude.size(); ++basis)
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

} // namespace qpp::examples::binary_search

