#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <complex>
#include <cstddef>
#include <limits>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "qpp/pbool.h"
#include "qpp/qint"
#include "qpp/qstruct.hpp"
#include "qpp/qvector"

namespace qpp::examples::two_pointers {

/// Determine whether a string is a palindrome ignoring punctuation and case.
inline bool valid_palindrome(std::string_view s) {
    std::size_t left = 0;
    std::size_t right = s.size();

    while (left < right) {
        while (left < right &&
               !std::isalnum(static_cast<unsigned char>(s[static_cast<std::size_t>(left)])))
            ++left;
        while (left < right &&
               !std::isalnum(static_cast<unsigned char>(s[static_cast<std::size_t>(right - 1)])))
            --right;
        if (left >= right)
            break;

        const unsigned char left_char =
            static_cast<unsigned char>(s[static_cast<std::size_t>(left)]);
        const unsigned char right_char =
            static_cast<unsigned char>(s[static_cast<std::size_t>(right - 1)]);
        if (std::tolower(left_char) != std::tolower(right_char))
            return false;

        ++left;
        --right;
    }

    return true;
}

/// Determine whether a string is a palindrome using the same rules as the
/// classical counterpart.
inline bool quantum_valid_palindrome(std::string_view s) {
    return valid_palindrome(s);
}

/// Probability wrapper describing confidence that the provided string is a
/// palindrome.
inline qpp::pbool palindrome_bias(std::string_view s) {
    const double probability = valid_palindrome(s) ? 1.0 : 0.0;
    return qpp::pbool{probability};
}


/// Collapse the logical axis representing the classical value of a quantum
/// integer.
inline int measure_value(const qint& value) {
    const auto stats = value.measure_axis(0U);
    if (stats.collapsed_value)
        return *stats.collapsed_value;
    return value.x_step()();
}

namespace detail {

/// Measure each value to obtain classical integers for downstream processing.
inline std::vector<int>
collapse_two_pointer_inputs(const qpp::qvector<qint>& values) {
    std::vector<int> collapsed;
    collapsed.reserve(values.size());

    for (const auto& value : values)
        collapsed.push_back(measure_value(value));

    return collapsed;
}

/// Lift a classical integer into a fresh quantum register with aligned axes.
inline qint make_quantum_value(int classical) {
    return qint{classical, classical, classical, classical};
}

/// Transform a classical triplet into its quantum representation.
inline std::array<qint, 3>
lift_three_sum_triplet(const std::array<int, 3>& triplet) {
    return {make_quantum_value(triplet[0]), make_quantum_value(triplet[1]),
            make_quantum_value(triplet[2])};
}

} // namespace detail

/// Enumerate all unique triplets that sum to zero over classical integers.
inline std::vector<std::array<int, 3>> three_sum(std::vector<int> nums) {
    std::vector<std::array<int, 3>> result;
    if (nums.size() < 3)
        return result;

    std::sort(nums.begin(), nums.end());

    for (std::size_t i = 0; i + 2 < nums.size(); ++i) {
        if (i > 0 && nums[i] == nums[i - 1])
            continue;

        std::size_t left = i + 1;
        std::size_t right = nums.size() - 1;
        while (left < right) {
            const long long current = static_cast<long long>(nums[i]) +
                                      static_cast<long long>(nums[left]) +
                                      static_cast<long long>(nums[right]);
            if (current < 0) {
                ++left;
            } else if (current > 0) {
                --right;
            } else {
                result.push_back({nums[i], nums[left], nums[right]});
                ++left;
                --right;
                while (left < right && nums[left] == nums[left - 1])
                    ++left;
                while (left < right && nums[right] == nums[right + 1])
                    --right;
            }
        }
    }

    return result;
}

/// Enumerate all unique triplets that sum to zero for quantum integers by
/// measuring inputs and lifting the classical results back into quantum
/// registers.
inline qpp::qvector<std::array<qint, 3>>
quantum_three_sum(const qpp::qvector<qint>& nums) {
    const auto collapsed = detail::collapse_two_pointer_inputs(nums);
    const auto classical = three_sum(collapsed);

    qpp::qvector<std::array<qint, 3>> lifted;
    lifted.reserve(classical.size());
    for (const auto& triplet : classical)
        lifted.push_back(detail::lift_three_sum_triplet(triplet));

    return lifted;
}

/// Compute the largest container area using classical integers.
inline int container_with_most_water(const std::vector<int>& height) {
    if (height.size() < 2)
        return 0;

    std::size_t left = 0;
    std::size_t right = height.size() - 1;
    int best = 0;

    while (left < right) {
        const int min_height = std::min(height[left], height[right]);
        const int width = static_cast<int>(right - left);
        best = std::max(best, min_height * width);

        if (height[left] < height[right])
            ++left;
        else
            --right;
    }

    return best;
}

/// Quantum wrapper that measures inputs then lifts the area back into a
/// freshly-allocated quantum register.
inline qint
quantum_container_with_most_water(const qpp::qvector<qint>& height) {
    const auto collapsed = detail::collapse_two_pointer_inputs(height);
    return detail::make_quantum_value(container_with_most_water(collapsed));
}

/// Build a register representing a uniform superposition of pointer positions.
inline qpp::qclass make_pointer_superposition(std::size_t length) {
    if (length == 0)
        return qpp::qclass{};

    std::size_t qubits = 0;
    while ((std::size_t{1} << qubits) < length)
        ++qubits;

    qpp::qclass reg(qubits);
    for (std::size_t q = 0; q < qubits; ++q)
        reg.apply_h(q);
    return reg;
}

/// Probability that a randomly sampled pair of pointers intersects the target
/// index.
inline qpp::pbool pointer_hit_probability(std::size_t length, std::size_t target) {
    if (length == 0 || target >= length)
        return qpp::pbool{0.0};

    const double numerator = static_cast<double>(target + 1) *
                              static_cast<double>(length - target);
    const double denominator =
        static_cast<double>(length) * static_cast<double>(length + 1) / 2.0;
    return qpp::pbool{numerator / denominator};
}

} // namespace qpp::examples::two_pointers

