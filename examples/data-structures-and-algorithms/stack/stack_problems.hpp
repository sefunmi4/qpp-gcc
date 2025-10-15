#pragma once

#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "qpp/pbool.h"
#include "qpp/qint"
#include "qpp/qvector"

namespace qpp::examples::stack {
namespace detail {

constexpr std::array<char, 6> kBracketSymbols{'(', ')', '[', ']', '{', '}'};

// Each bracket token is mapped onto a unique set of axis orientations so that a
// qint can faithfully round-trip through encode/decode operations. The mapping
// is intentionally documented here to keep the classical reference helpers and
// the quantum-facing implementation in sync:
//
//   * The x-axis distinguishes opening (-1) from closing (+1) brackets.
//   * The y- and z-axes identify the bracket family:
//       - Parentheses  -> (y = -1, z = -1)
//       - Square       -> (y = +1, z = -1)
//       - Curly        -> (y = -1, z = +1)
//   * The t-axis remains fixed at -1 for every symbol to maintain a consistent
//     orientation history in the legacy backend.
//
// This layout yields six distinct patterns—one for each supported bracket
// symbol—while using only three measurement axes for classification.
constexpr std::array<std::array<int, 4>, kBracketSymbols.size()>
    kBracketAxisEncodings{{{{-1, -1, -1, -1}},  // '('
                           {{+1, -1, -1, -1}},  // ')'
                           {{-1, +1, -1, -1}},  // '['
                           {{+1, +1, -1, -1}},  // ']'
                           {{-1, -1, +1, -1}},  // '{'
                           {{+1, -1, +1, -1}}}}; // '}'

[[nodiscard]] constexpr bool is_open_symbol(char symbol) noexcept {
    return symbol == '(' || symbol == '[' || symbol == '{';
}

[[nodiscard]] constexpr bool is_supported_symbol(char symbol) noexcept {
    for (char candidate : kBracketSymbols)
        if (candidate == symbol)
            return true;
    return false;
}

[[nodiscard]] constexpr bool is_matching_pair(char open_symbol,
                                              char close_symbol) noexcept {
    return (open_symbol == '(' && close_symbol == ')') ||
           (open_symbol == '[' && close_symbol == ']') ||
           (open_symbol == '{' && close_symbol == '}');
}

[[nodiscard]] constexpr int encode_symbol(char symbol) noexcept {
    for (std::size_t index = 0; index < kBracketSymbols.size(); ++index)
        if (kBracketSymbols[index] == symbol)
            return static_cast<int>(index);
    return -1;
}

[[nodiscard]] constexpr const std::array<int, 4>&
axes_for_token(std::size_t token) noexcept {
    return kBracketAxisEncodings[token];
}

[[nodiscard]] inline int decode_token_from_axes(const std::array<int, 4>& axes) {
    for (std::size_t index = 0; index < kBracketAxisEncodings.size(); ++index)
        if (kBracketAxisEncodings[index] == axes)
            return static_cast<int>(index);
    return -1;
}

[[nodiscard]] inline std::array<int, 4> sample_axes(const qint& value) {
    return {value.x_step().sample(), value.y_step().sample(),
            value.z_step().sample(), value.t_step().sample()};
}

[[nodiscard]] constexpr char decode_symbol(int token) noexcept {
    return (token >= 0 && token < static_cast<int>(kBracketSymbols.size()))
               ? kBracketSymbols[static_cast<std::size_t>(token)]
               : '\0';
}

template <typename FetchSymbol>
[[nodiscard]] inline bool validate_sequence(std::size_t length,
                                            FetchSymbol&& fetch_symbol) {
    std::vector<char> symbols;
    symbols.reserve(length);

    for (std::size_t index = 0; index < length; ++index) {
        const char symbol = fetch_symbol(index);
        if (!is_supported_symbol(symbol))
            return false;

        if (is_open_symbol(symbol)) {
            symbols.push_back(symbol);
            continue;
        }

        if (symbols.empty() || !is_matching_pair(symbols.back(), symbol))
            return false;

        symbols.pop_back();
    }

    return symbols.empty();
}

} // namespace detail


/// Determine whether a bracket string contains a valid sequence of symbols.
inline bool is_valid_parentheses(std::string_view s) {
    return detail::validate_sequence(s.size(),
                                     [s](std::size_t index) {
                                         return s[index];
                                     });
}

/// Determine whether a sequence of encoded bracket tokens is valid.
inline bool is_valid_parentheses(const std::qvector<qint>& sequence) {
    return detail::validate_sequence(sequence.size(),
                                     [&](std::size_t index) {
                                         const auto measured_axes =
                                             detail::sample_axes(sequence[index]);
                                         const int token =
                                             detail::decode_token_from_axes(
                                                 measured_axes);
                                         return detail::decode_symbol(token);
                                     });
}

/// Convenience wrapper that mirrors the quantum-prefixed API used by the sample.
inline bool quantum_is_valid_parentheses(const std::qvector<qint>& sequence) {
    return is_valid_parentheses(sequence);
}

/// Express confidence in the validity of the provided bracket string.
inline qpp::pbool parentheses_confidence(std::string_view s) {
    return qpp::pbool{is_valid_parentheses(s) ? 1.0 : 0.0};
}

/// Convert a bracket string into a quantum-friendly encoding of bracket tokens.
inline std::qvector<qint> encode_parentheses_sequence(std::string_view s) {
    std::qvector<qint> encoded;
    encoded.reserve(s.size());

    for (char symbol : s) {
        const int token = detail::encode_symbol(symbol);
        if (token < 0)
            throw std::invalid_argument(
                "encode_parentheses_sequence: unsupported bracket symbol");
        const auto& axes =
            detail::axes_for_token(static_cast<std::size_t>(token));
        encoded.emplace_back(axes[0], axes[1], axes[2], axes[3]);
    }

    return encoded;
}

/// Convert an encoded bracket sequence back into its string representation.
inline std::string decode_parentheses_sequence(const std::qvector<qint>& sequence) {
    std::string decoded;
    decoded.reserve(sequence.size());

    for (const auto& token : sequence) {
        const auto measured_axes = detail::sample_axes(token);
        const int decoded_token = detail::decode_token_from_axes(measured_axes);
        const char symbol = detail::decode_symbol(decoded_token);
        if (!detail::is_supported_symbol(symbol))
            throw std::invalid_argument(
                "decode_parentheses_sequence: invalid bracket token");
        decoded.push_back(symbol);
    }

    return decoded;
}

} // namespace qpp::examples::stack

