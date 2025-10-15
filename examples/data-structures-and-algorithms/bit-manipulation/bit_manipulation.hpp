#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "qpp/pbool.h"
#include "qpp/qint"
#include "qpp/qstruct.hpp"
#include "qpp/qvector"

namespace qpp::examples::bit_manipulation {

namespace detail {

inline constexpr unsigned quantum_bit_width = 4U;
inline constexpr std::uint32_t quantum_mask = (1u << quantum_bit_width) - 1u;

inline int orientation_to_bit(int orientation) noexcept { return orientation > 0 ? 1 : 0; }

inline int bit_to_orientation(unsigned bit) noexcept { return bit ? 1 : -1; }

inline std::array<int, quantum_bit_width> sample_axes(const qint& value) {
    return {value.x_step().sample(), value.y_step().sample(), value.z_step().sample(),
            value.t_step().sample()};
}

inline std::uint32_t axes_to_bits(const std::array<int, quantum_bit_width>& axes) noexcept {
    std::uint32_t bits = 0;
    for (int orientation : axes) {
        bits <<= 1U;
        bits |= static_cast<std::uint32_t>(orientation_to_bit(orientation));
    }
    return bits;
}

inline std::uint32_t collapse_to_bits(const qint& value) {
    return axes_to_bits(sample_axes(value));
}

inline std::array<int, quantum_bit_width> bits_to_axes(std::uint32_t bits) noexcept {
    std::array<int, quantum_bit_width> axes{};
    for (std::size_t axis = 0; axis < axes.size(); ++axis) {
        const std::size_t shift = axes.size() - axis - 1U;
        axes[axis] = bit_to_orientation((bits >> shift) & 0x1u);
    }
    return axes;
}

inline qint build_from_bits(std::uint32_t bits) {
    const auto axes = bits_to_axes(bits & quantum_mask);
    return qint(axes[0], axes[1], axes[2], axes[3]);
}

inline int collapse_to_signed(const qint& value) {
    const std::uint32_t bits = collapse_to_bits(value);
    const std::uint32_t sign_mask = 1u << (quantum_bit_width - 1U);
    if (bits & sign_mask) {
        const std::int32_t extended = static_cast<std::int32_t>(bits | (~quantum_mask));
        return static_cast<int>(extended);
    }
    return static_cast<int>(bits);
}

inline qint build_from_signed(int value) {
    const std::uint32_t bits = static_cast<std::uint32_t>(value) & quantum_mask;
    return build_from_bits(bits);
}

inline std::vector<int> collapse_vector(const std::qvector<qint>& values) {
    std::vector<int> result;
    result.reserve(values.size());
    for (const auto& value : values)
        result.push_back(collapse_to_signed(value));
    return result;
}

inline std::qvector<qint> build_vector(const std::vector<int>& values) {
    std::qvector<qint> result;
    result.reserve(values.size());
    for (int value : values)
        result.push_back(build_from_signed(value));
    return result;
}

inline std::uint32_t reverse_bits_with_width(std::uint32_t bits, unsigned width) noexcept {
    std::uint32_t reversed = 0;
    for (unsigned i = 0; i < width; ++i) {
        reversed <<= 1U;
        reversed |= (bits >> i) & 0x1u;
    }
    return reversed;
}

} // namespace detail

inline constexpr unsigned quantum_bit_width = detail::quantum_bit_width;

/// Count the number of set bits for a classical integer.
inline int hamming_weight(std::uint32_t value) {
    int count = 0;
    while (value != 0U) {
        value &= value - 1U;
        ++count;
    }
    return count;
}

/// Count the number of set bits for a quantum integer by collapsing to the classical domain.
inline int hamming_weight(const qint& value) {
    return hamming_weight(detail::collapse_to_bits(value));
}

/// Compute population counts for the range [0, n] (classical).
inline std::vector<int> counting_bits(int n) {
    if (n < 0)
        return {};

    std::vector<int> result(static_cast<std::size_t>(n) + 1);
    for (int i = 0; i <= n; ++i)
        result[static_cast<std::size_t>(i)] =
            hamming_weight(static_cast<std::uint32_t>(i));
    return result;
}

/// Compute population counts for the range [0, n] and encode them as quantum integers.
inline std::qvector<qint> quantum_counting_bits(int n) {
    return detail::build_vector(counting_bits(n));
}

/// Reverse the bits of a 32-bit classical value.
inline std::uint32_t reverse_bits(std::uint32_t value) {
    value = ((value & 0x55555555u) << 1U) | ((value >> 1U) & 0x55555555u);
    value = ((value & 0x33333333u) << 2U) | ((value >> 2U) & 0x33333333u);
    value = ((value & 0x0F0F0F0Fu) << 4U) | ((value >> 4U) & 0x0F0F0F0Fu);
    value = ((value & 0x00FF00FFu) << 8U) | ((value >> 8U) & 0x00FF00FFu);
    value = (value << 16U) | (value >> 16U);
    return value;
}

/// Reverse the bits stored within a quantum integer.
inline qint reverse_bits(const qint& value) {
    const std::uint32_t bits = detail::collapse_to_bits(value);
    const std::uint32_t reversed =
        detail::reverse_bits_with_width(bits, detail::quantum_bit_width);
    return detail::build_from_bits(reversed);
}

/// Return the missing value in the classical variant of the problem.
inline int missing_number(const std::vector<int>& nums) {
    int xor_accumulator = static_cast<int>(nums.size());
    for (std::size_t i = 0; i < nums.size(); ++i) {
        xor_accumulator ^= static_cast<int>(i);
        xor_accumulator ^= nums[i];
    }
    return xor_accumulator;
}

/// Return the missing value in the quantum variant of the problem.
inline qint missing_number(const std::qvector<qint>& nums) {
    const auto classical = detail::collapse_vector(nums);
    return detail::build_from_signed(missing_number(classical));
}

/// Compute the bitwise sum of two classical integers without using the + operator.
inline int sum_two_integers(int a, int b) {
    while (b != 0) {
        const unsigned carry = static_cast<unsigned>(a & b) << 1U;
        a ^= b;
        b = static_cast<int>(carry);
    }
    return a;
}

/// Compute the bitwise sum of two quantum integers.
inline qint sum_two_integers(const qint& a, const qint& b) {
    const int lhs = detail::collapse_to_signed(a);
    const int rhs = detail::collapse_to_signed(b);
    return detail::build_from_signed(sum_two_integers(lhs, rhs));
}

/// Convenience wrapper for converting a classical unsigned value into a quantum integer.
inline qint make_quantum_from_uint32(std::uint32_t value) {
    return detail::build_from_bits(value);
}

/// Convenience wrapper for converting a classical signed value into a quantum integer.
inline qint make_quantum_from_int(int value) {
    return detail::build_from_signed(value);
}

/// Collapse a quantum integer into its unsigned classical representation.
inline std::uint32_t collapse_quantum_to_uint32(const qint& value) {
    return detail::collapse_to_bits(value);
}

/// Collapse a quantum integer into its signed classical representation.
inline int collapse_quantum_to_int(const qint& value) {
    return detail::collapse_to_signed(value);
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

/// Convenience wrappers mirroring the quantum-prefixed APIs used by the sample.
inline int quantum_hamming_weight(const qint& value) { return hamming_weight(value); }

inline qint quantum_reverse_bits(const qint& value) { return reverse_bits(value); }

inline qint quantum_missing_number(const std::qvector<qint>& nums) {
    return missing_number(nums);
}

inline qint quantum_sum_two_integers(const qint& a, const qint& b) {
    return sum_two_integers(a, b);
}

} // namespace qpp::examples::bit_manipulation

