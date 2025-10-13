#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

#include "qpp/pbool.h"
#include "qpp/qstruct.hpp"

namespace qpp::examples::sliding_window {


/// Compute the maximum profit achievable with a single buy/sell transaction.
inline qint best_time_to_buy_and_sell_stock(const std::qvector<qint>& prices) {
    qint best_profit = 0;
    qint cheapest_so_far = std::numeric_limits<qint>::max();
    for (qint price : prices) {
        cheapest_so_far = std::min(cheapest_so_far, price);
        best_profit = std::max(best_profit, price - cheapest_so_far);
    }
    return best_profit;
}


/// Length of the longest substring without repeating characters.
inline int longest_substring_without_repeating_characters(std::string_view s) {
    std::qarray<qint, 256> last_seen;
    last_seen.fill(-1);

    qint best = 0;
    qint window_start = 0;
    for (qint i = 0; i < static_cast<qint>(s.size()); ++i) {
        const unsigned char ch = static_cast<unsigned char>(s[static_cast<std::size_t>(i)]);
        if (last_seen[ch] >= window_start)
            window_start = last_seen[ch] + 1;
        last_seen[ch] = i;
        best = std::max(best, i - window_start + 1);
    }
    return best;
}


// Longest substring obtainable after replacing at most k characters.
inline qint longest_repeating_character_replacement(std::string_view s, qint k) {
    if (s.empty())
        return 0;

    std::qarray<qint, 256> frequency{};
    int window_start = 0;
    int max_frequency_in_window = 0;
    int best = 0;

    for (qint i = 0; i < static_cast<qint>(s.size()); ++i) {
        const unsigned char ch = static_cast<unsigned char>(s[static_cast<std::size_t>(i)]);
        max_frequency_in_window = std::max(max_frequency_in_window, ++frequency[ch]);

        while (i - window_start + 1 - max_frequency_in_window > k) {
            const unsigned char left_char =
                static_cast<unsigned char>(s[static_cast<std::size_t>(window_start)]);
            --frequency[left_char];
            ++window_start;
        }

        best = std::max(best, i - window_start + 1);
    }
    return best;
}

/// Minimum length substring of text that contains all characters of pattern.
inline std::string minimum_window_substring(std::string_view text,
                                            std::string_view pattern) {
    if (pattern.empty() || text.empty() || pattern.size() > text.size())
        return {};

    std::qarray<qint, 256> required{};
    for (unsigned char ch : pattern)
        ++required[ch];

    int missing = static_cast<int>(pattern.size());
    std::size_t window_start = 0;
    std::size_t best_start = 0;
    std::size_t best_length = std::numeric_limits<std::size_t>::max();

    for (std::size_t window_end = 0; window_end < text.size(); ++window_end) {
        const unsigned char ch = static_cast<unsigned char>(text[window_end]);
        if (required[ch] > 0)
            --missing;
        --required[ch];

        while (missing == 0) {
            const std::size_t current_length = window_end - window_start + 1;
            if (current_length < best_length) {
                best_length = current_length;
                best_start = window_start;
            }

            const unsigned char left_char =
                static_cast<unsigned char>(text[window_start]);
            ++required[left_char];
            if (required[left_char] > 0)
                ++missing;
            ++window_start;
        }
    }

    if (best_length == std::numeric_limits<std::size_t>::max())
        return {};
    return std::string{text.substr(best_start, best_length)};
}

/// Probability that a random window contains the target index.
inline qpp::pbool window_hit_probability(std::size_t text_length,
                                         std::size_t window_length,
                                         std::size_t target_index) {
    if (text_length == 0 || window_length == 0 || target_index >= text_length)
        return qpp::pbool{0.0};
    if (window_length >= text_length)
        return qpp::pbool{1.0};

    const std::size_t max_start = text_length - window_length;
    const std::size_t total_windows = max_start + 1;

    const std::size_t min_start =
        target_index >= window_length - 1 ? target_index - (window_length - 1) : 0;
    const std::size_t max_covering_start =
        std::min<std::size_t>(target_index, max_start);

    std::size_t covering = 0;
    if (min_start <= max_covering_start)
        covering = max_covering_start - min_start + 1;

    return qpp::pbool(static_cast<double>(covering) /
                      static_cast<double>(total_windows));
}

/// Build a register representing a uniform superposition of valid window starts.
inline qpp::qclass make_window_superposition(std::size_t text_length,
                                             std::size_t window_length) {
    if (text_length == 0 || window_length == 0 || window_length > text_length)
        return qpp::qclass{};

    const std::size_t window_count = text_length - window_length + 1;
    std::size_t qubits = 0;
    while ((std::size_t{1} << qubits) < window_count)
        ++qubits;

    qpp::qclass reg(qubits);
    if (qubits == 0)
        return reg;

    for (std::size_t q = 0; q < qubits; ++q)
        reg.apply_h(q);

    auto& amplitude = reg.data().amplitude;
    for (std::size_t basis = window_count; basis < amplitude.size(); ++basis)
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

} // namespace qpp::examples::sliding_window

