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

/// Determine whether a string is a palindrome using the same rules as the
/// classical counterpart.
inline bool quantum_valid_palindrome(std::string_view s) {
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

/// Probability wrapper describing confidence that the provided string is a
/// palindrome.
inline qpp::pbool palindrome_bias(std::string_view s) {
    return qpp::pbool{valid_palindrome(s) ? 1.0 : 0.0};
}


/// Enumerate all unique triplets that sum to zero over quantum integers.
inline std::vector<std::array<qint, 3>> three_sum(std::qvector<qint> nums) {
    if (nums.size() < 3)
        return {};

    std::sort(nums.begin(), nums.end());

    std::vector<std::array<qint, 3>> result;
    for (std::size_t i = 0; i < nums.size(); ++i) {
        if (i > 0 && nums[i] == nums[i - 1])
            continue;

        std::size_t left = i + 1;
        std::size_t right = nums.size() - 1;
        while (left < right) {
            const qint current = nums[i] + nums[left] + nums[right];
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


/// Compute the largest container area using quantum integers.
inline qint container_with_most_water(const std::qvector<qint>& height) {
    if (height.size() < 2)
        return 0;

    std::size_t left = 0;
    std::size_t right = height.size() - 1;
    qint best = 0;

    while (left < right) {
        const qint min_height = std::min(height[left], height[right]);
        const qint width = static_cast<qint>(right - left);
        best = std::max(best, min_height * width);

        if (height[left] < height[right])
            ++left;
        else
            --right;
    }

    return best;
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

