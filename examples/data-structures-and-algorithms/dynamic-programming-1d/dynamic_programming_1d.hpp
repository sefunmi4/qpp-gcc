#pragma once

#include <algorithm>
#include <cstddef>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "qpp/pbool.h"
#include "qpp/quantum_dp.hpp"

namespace qpp::examples::dynamic_programming_1d {

/// Compute the number of distinct ways to climb a staircase with n steps where
/// you may climb either 1 or 2 steps at a time.
inline qpp::qdp::quantum_scalar<int>
climb_stairs(const qpp::qdp::quantum_scalar<int>& n) {
    const int steps = n.value();
    if (steps <= 1)
        return qpp::qdp::quantum_scalar<int>(1);
    int prev2 = 1;
    int prev1 = 1;
    for (int i = 2; i <= steps; ++i) {
        const int current = prev1 + prev2;
        prev2 = prev1;
        prev1 = current;
    }
    return qpp::qdp::quantum_scalar<int>(prev1);
}

/// Classic "House Robber" solution: maximise the sum of non-adjacent houses.
inline qpp::qdp::quantum_scalar<int>
house_robber(const qpp::qdp::quantum_sequence<int>& nums) {
    int rob_prev = 0;
    int skip_prev = 0;
    for (int value : nums) {
        const int rob_current = skip_prev + value;
        skip_prev = std::max(skip_prev, rob_prev);
        rob_prev = rob_current;
    }
    return qpp::qdp::quantum_scalar<int>(std::max(rob_prev, skip_prev));
}

namespace detail {
inline qpp::qdp::quantum_scalar<int>
linear_house_robber(const qpp::qdp::quantum_sequence<int>& nums) {
    int rob_prev = 0;
    int skip_prev = 0;
    for (int value : nums) {
        const int rob_current = skip_prev + value;
        skip_prev = std::max(skip_prev, rob_prev);
        rob_prev = rob_current;
    }
    return qpp::qdp::quantum_scalar<int>(std::max(rob_prev, skip_prev));
}
} // namespace detail

/// House Robber II where the first and last houses are adjacent.
inline qpp::qdp::quantum_scalar<int>
house_robber_ii(const qpp::qdp::quantum_sequence<int>& nums) {
    if (nums.empty())
        return qpp::qdp::quantum_scalar<int>(0);
    if (nums.size() == 1)
        return qpp::qdp::quantum_scalar<int>(nums[0]);
    const qpp::qdp::quantum_sequence<int> left(nums.begin(), nums.end() - 1);
    const qpp::qdp::quantum_sequence<int> right(nums.begin() + 1, nums.end());
    const auto left_result = detail::linear_house_robber(left);
    const auto right_result = detail::linear_house_robber(right);
    return qpp::qdp::quantum_scalar<int>(
        std::max(left_result.value(), right_result.value()));
}

/// Expand-around-centre helper for palindrome routines.
inline qpp::qdp::quantum_index_range expand_from_center(
    const qpp::qdp::quantum_string& s, qpp::qdp::quantum_scalar<int> left,
    qpp::qdp::quantum_scalar<int> right) {
    int l = left.value();
    int r = right.value();
    const std::string_view view = s.view();
    while (l >= 0 && r < static_cast<int>(view.size()) &&
           view[static_cast<std::size_t>(l)] ==
               view[static_cast<std::size_t>(r)]) {
        --l;
        ++r;
    }
    return qpp::qdp::quantum_index_range(l + 1, r - 1);
}

/// Return the longest palindromic substring of the input.
inline qpp::qdp::quantum_string
longest_palindromic_substring(const qpp::qdp::quantum_string& s) {
    if (s.empty())
        return qpp::qdp::quantum_string();

    int best_start = 0;
    int best_end = 0;
    const std::string_view view = s.view();
    for (int i = 0; i < static_cast<int>(view.size()); ++i) {
        const auto odd =
            expand_from_center(s, qpp::qdp::quantum_scalar<int>(i),
                               qpp::qdp::quantum_scalar<int>(i));
        if (odd.length() > best_end - best_start) {
            best_start = odd.left.value();
            best_end = odd.right.value();
        }

        const auto even = expand_from_center(
            s, qpp::qdp::quantum_scalar<int>(i),
            qpp::qdp::quantum_scalar<int>(i + 1));
        if (even.length() > best_end - best_start) {
            best_start = even.left.value();
            best_end = even.right.value();
        }
    }
    return qpp::qdp::quantum_string(view.substr(
        static_cast<std::size_t>(best_start),
        static_cast<std::size_t>(best_end - best_start + 1)));
}

/// Count the total number of palindromic substrings in the input.
inline qpp::qdp::quantum_scalar<int>
palindromic_substring_count(const qpp::qdp::quantum_string& s) {
    int total = 0;
    const std::string_view view = s.view();
    for (int center = 0; center < static_cast<int>(view.size()); ++center) {
        int left = center;
        int right = center;
        while (left >= 0 && right < static_cast<int>(view.size()) &&
               view[static_cast<std::size_t>(left)] ==
                   view[static_cast<std::size_t>(right)]) {
            ++total;
            --left;
            ++right;
        }
        left = center;
        right = center + 1;
        while (left >= 0 && right < static_cast<int>(view.size()) &&
               view[static_cast<std::size_t>(left)] ==
                   view[static_cast<std::size_t>(right)]) {
            ++total;
            --left;
            ++right;
        }
    }
    return qpp::qdp::quantum_scalar<int>(total);
}

/// Count the number of ways to decode a numeric string mapping to letters.
inline qpp::qdp::quantum_scalar<int>
decode_ways(const qpp::qdp::quantum_string& s) {
    const std::string_view view = s.view();
    if (view.empty() || view.front() == '0')
        return qpp::qdp::quantum_scalar<int>(0);

    int prev2 = 1; // ways to decode up to i-2
    int prev1 = 1; // ways to decode up to i-1
    for (std::size_t i = 1; i < view.size(); ++i) {
        int current = 0;
        if (view[i] != '0')
            current += prev1;
        const int two_digit = (view[i - 1] - '0') * 10 + (view[i] - '0');
        if (two_digit >= 10 && two_digit <= 26)
            current += prev2;
        if (current == 0)
            return qpp::qdp::quantum_scalar<int>(0);
        prev2 = prev1;
        prev1 = current;
    }
    return qpp::qdp::quantum_scalar<int>(prev1);
}

/// Minimum number of coins required to compose the given amount.
inline qpp::qdp::quantum_scalar<int>
coin_change(const qpp::qdp::quantum_sequence<int>& coins,
            const qpp::qdp::quantum_scalar<int>& amount) {
    const int target = amount.value();
    if (target < 0)
        return qpp::qdp::quantum_scalar<int>(-1);
    qpp::qdp::quantum_sequence<int> dp(
        static_cast<std::size_t>(target) + 1, target + 1);
    dp[0] = 0;
    for (int coin : coins) {
        if (coin <= 0)
            continue;
        for (int value = coin; value <= target; ++value) {
            const int prev = dp[static_cast<std::size_t>(value - coin)];
            if (prev != target + 1)
                dp[static_cast<std::size_t>(value)] =
                    std::min(dp[static_cast<std::size_t>(value)], prev + 1);
        }
    }
    dp.refresh_state();
    const int result = dp[static_cast<std::size_t>(target)];
    return result > target ? qpp::qdp::quantum_scalar<int>(-1)
                           : qpp::qdp::quantum_scalar<int>(result);
}

/// Maximum product obtainable from a contiguous subarray.
inline qpp::qdp::quantum_scalar<int>
maximum_product_subarray(const qpp::qdp::quantum_sequence<int>& nums) {
    if (nums.empty())
        return qpp::qdp::quantum_scalar<int>(0);
    int best = nums[0];
    int max_ending_here = nums[0];
    int min_ending_here = nums[0];
    for (std::size_t i = 1; i < nums.size(); ++i) {
        if (nums[i] < 0)
            std::swap(max_ending_here, min_ending_here);
        max_ending_here = std::max(nums[i], max_ending_here * nums[i]);
        min_ending_here = std::min(nums[i], min_ending_here * nums[i]);
        best = std::max(best, max_ending_here);
    }
    return qpp::qdp::quantum_scalar<int>(best);
}

/// Determine whether the input string can be segmented into dictionary words.
inline qpp::qdp::quantum_scalar<bool>
word_break(const qpp::qdp::quantum_string& s,
           const qpp::qdp::quantum_sequence<std::string>& word_dict) {
    std::unordered_set<std::string> dict(word_dict.begin(), word_dict.end());
    qpp::qdp::quantum_sequence<bool> dp(s.classical().size() + 1, false);
    dp[0] = true;
    const std::string_view view = s.view();
    for (std::size_t end = 1; end <= view.size(); ++end) {
        for (std::size_t start = 0; start < end; ++start) {
            if (dp[start] &&
                dict.find(std::string{view.substr(start, end - start)}) !=
                    dict.end()) {
                dp[end] = true;
                break;
            }
        }
    }
    dp.refresh_state();
    return qpp::qdp::quantum_scalar<bool>(dp[view.size()]);
}

/// Encode the deterministic word break result as a quantum-inspired probability.
inline qpp::pbool quantum_word_break(
    const qpp::qdp::quantum_string& s,
    const qpp::qdp::quantum_sequence<std::string>& word_dict) {
    const auto result = word_break(s, word_dict);
    return qpp::pbool(result.value() ? 1.0 : 0.0);
}

/// Length of the longest increasing subsequence using patience sorting.
inline qpp::qdp::quantum_scalar<int>
longest_increasing_subsequence(const qpp::qdp::quantum_sequence<int>& nums) {
    std::vector<int> tails;
    tails.reserve(nums.size());
    for (int value : nums) {
        auto it = std::lower_bound(tails.begin(), tails.end(), value);
        if (it == tails.end())
            tails.push_back(value);
        else
            *it = value;
    }
    return qpp::qdp::quantum_scalar<int>(static_cast<int>(tails.size()));
}

} // namespace qpp::examples::dynamic_programming_1d

