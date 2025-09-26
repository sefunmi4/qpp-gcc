#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "qpp/pbool.h"
#include "qpp/qint"
#include "qpp/qstruct.hpp"
#include "qpp/qvector"

namespace qpp::examples::bit_manipulation {

/// Count the number of set bits (population count) in the provided value.
inline int hamming_weight(std::uint32_t value) {
    int count = 0;
    while (value != 0U) {
        value &= value - 1U;
        ++count;
    }
    return count;
}

/// Count the number of set bits for a quantum integer by delegating to the
/// classical counterpart.
inline int quantum_hamming_weight(const qint& value) {
    return hamming_weight(static_cast<std::uint32_t>(static_cast<int>(value)));
}

/// Build the array containing the population count of each value in [0, n].
inline std::vector<int> counting_bits(int n) {
    if (n < 0)
        return {};

    std::vector<int> result(static_cast<std::size_t>(n) + 1, 0);
    for (int i = 1; i <= n; ++i)
        result[static_cast<std::size_t>(i)] =
            result[static_cast<std::size_t>(i >> 1)] + (i & 1);
    return result;
}

/// Compute population counts for the range [0, n] and encode them as quantum
/// integers.
inline std::qvector<qint> quantum_counting_bits(int n) {
    const auto classical = counting_bits(n);
    std::qvector<qint> result;
    result.reserve(classical.size());
    for (int value : classical)
        result.push_back(static_cast<qint>(value));
    return result;
}

/// Reverse the bits of a 32-bit unsigned integer.
inline std::uint32_t reverse_bits(std::uint32_t value) {
    value = ((value & 0x55555555u) << 1U) | ((value >> 1U) & 0x55555555u);
    value = ((value & 0x33333333u) << 2U) | ((value >> 2U) & 0x33333333u);
    value = ((value & 0x0F0F0F0Fu) << 4U) | ((value >> 4U) & 0x0F0F0F0Fu);
    value = ((value & 0x00FF00FFu) << 8U) | ((value >> 8U) & 0x00FF00FFu);
    value = (value << 16U) | (value >> 16U);
    return value;
}

/// Reverse the bits of a quantum integer treated as a 32-bit value.
inline qint quantum_reverse_bits(const qint& value) {
    return static_cast<qint>(reverse_bits(static_cast<std::uint32_t>(static_cast<int>(value))));
}

/// Given an array containing the range [0, n] with a missing element, return
/// the missing value.
inline int missing_number(const std::vector<int>& nums) {
    int xor_accumulator = static_cast<int>(nums.size());
    for (std::size_t i = 0; i < nums.size(); ++i) {
        xor_accumulator ^= static_cast<int>(i);
        xor_accumulator ^= nums[i];
    }
    return xor_accumulator;
}

/// Return the missing value in the quantum variant of the problem.
inline qint quantum_missing_number(const std::qvector<qint>& nums) {
    std::vector<int> classical;
    classical.reserve(nums.size());
    for (const auto& value : nums)
        classical.push_back(static_cast<int>(value));
    return static_cast<qint>(missing_number(classical));
}

/// Compute the sum of two integers using bitwise operations only.
inline int sum_two_integers(int a, int b) {
    while (b != 0) {
        const int carry = (a & b) << 1U;
        a ^= b;
        b = carry;
    }
    return a;
}

/// Compute the bitwise sum of two quantum integers.
inline qint quantum_sum_two_integers(const qint& a, const qint& b) {
    return static_cast<qint>(sum_two_integers(static_cast<int>(a), static_cast<int>(b)));
}

/// Probability wrapper describing whether a value has even parity.
inline qpp::pbool even_parity_probability(std::uint32_t value) {
    return qpp::pbool{(hamming_weight(value) % 2 == 0) ? 1.0 : 0.0};
}

/// Prepare a register encoding a uniform superposition across the provided
/// number of bits.
inline qpp::qclass make_uniform_bit_superposition(std::size_t bit_width) {
    qpp::qclass reg(bit_width);
    for (std::size_t q = 0; q < bit_width; ++q)
        reg.apply_h(q);
    return reg;
}

} // namespace qpp::examples::bit_manipulation

