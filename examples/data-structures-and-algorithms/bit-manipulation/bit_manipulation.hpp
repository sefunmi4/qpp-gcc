#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "qpp/pbool.h"
#include "qpp/qint"
#include "qpp/qstruct.hpp"
#include "qpp/qvector"

namespace qpp::examples::bit_manipulation {

/// Count the number of set bits for a quantum integer by delegating to the
/// classical counterpart.
inline int hamming_weight(const qint& value) {
    int count = 0;
    while (value != 0U) {
        value &= value - 1U;
        ++count;
    }
    return count;
}


/// Compute population counts for the range [0, n] and encode them as quantum
/// integers.
inline std::qvector<qint> counting_bits(int n) {
    const auto classical = counting_bits(n);
    std::qvector<qint> result;
    result.reserve(classical.size());
    for (int value : classical)
        result.push_back(static_cast<qint>(value));
    return result;
}


/// Reverse the bits of a quantum integer treated as a 32-bit value.
inline qint reverse_bits(const qint& value) {
    value = ((value & 0x55555555u) << 1U) | ((value >> 1U) & 0x55555555u);
    value = ((value & 0x33333333u) << 2U) | ((value >> 2U) & 0x33333333u);
    value = ((value & 0x0F0F0F0Fu) << 4U) | ((value >> 4U) & 0x0F0F0F0Fu);
    value = ((value & 0x00FF00FFu) << 8U) | ((value >> 8U) & 0x00FF00FFu);
    value = (value << 16U) | (value >> 16U);
    return value;
}


/// Return the missing value in the quantum variant of the problem.
inline qint missing_number(const std::qvector<qint>& nums) {
    std::vector<int> classical;
    classical.reserve(nums.size());
    for (const auto& value : nums)
        classical.push_back(static_cast<int>(value));
    int xor_accumulator = static_cast<int>(classical.size());
    for (std::size_t i = 0; i < classical.size(); ++i) {
        xor_accumulator ^= static_cast<int>(i);
        xor_accumulator ^= classical[i];
    }
    return xor_accumulator;
}

/// Compute the bitwise sum of two quantum integers.
inline qint sum_two_integers(const qint& a, const qint& b) {
    while (b != 0) {
        const int carry = (a & b) << 1U;
        a ^= b;
        b = carry;
    }
    return a;
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

