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

namespace qpp::examples::dynamic_programming_1d {

/// Compute the number of distinct ways to climb a staircase with n steps where
/// you may climb either 1 or 2 steps at a time.
inline int climb_stairs(int n) {
    if (n <= 1)
        return 1;
    int prev2 = 1;
    int prev1 = 1;
    for (int i = 2; i <= n; ++i) {
        const int current = prev1 + prev2;
        prev2 = prev1;
        prev1 = current;
    }
    return prev1;
}

/// Classic "House Robber" solution: maximise the sum of non-adjacent houses.
inline int house_robber(const std::vector<int>& nums) {
    int rob_prev = 0;
    int skip_prev = 0;
    for (int value : nums) {
        const int rob_current = skip_prev + value;
        skip_prev = std::max(skip_prev, rob_prev);
        rob_prev = rob_current;
    }
    return std::max(rob_prev, skip_prev);
}

namespace detail {
inline int linear_house_robber(const std::vector<int>& nums) {
    int rob_prev = 0;
    int skip_prev = 0;
    for (int value : nums) {
        const int rob_current = skip_prev + value;
        skip_prev = std::max(skip_prev, rob_prev);
        rob_prev = rob_current;
    }
    return std::max(rob_prev, skip_prev);
}
} // namespace detail

/// House Robber II where the first and last houses are adjacent.
inline int house_robber_ii(const std::vector<int>& nums) {
    if (nums.empty())
        return 0;
    if (nums.size() == 1)
        return nums.front();
    const std::vector<int> left(nums.begin(), nums.end() - 1);
    const std::vector<int> right(nums.begin() + 1, nums.end());
    return std::max(detail::linear_house_robber(left),
                    detail::linear_house_robber(right));
}

/// Expand-around-centre helper for palindrome routines.
inline std::pair<int, int> expand_from_center(std::string_view s, int left,
                                              int right) {
    while (left >= 0 && right < static_cast<int>(s.size()) &&
           s[static_cast<std::size_t>(left)] ==
               s[static_cast<std::size_t>(right)]) {
        --left;
        ++right;
    }
    return {left + 1, right - 1};
}

/// Return the longest palindromic substring of the input.
inline std::string longest_palindromic_substring(std::string_view s) {
    if (s.empty())
        return {};

    int best_start = 0;
    int best_end = 0;
    for (int i = 0; i < static_cast<int>(s.size()); ++i) {
        const auto [odd_start, odd_end] = expand_from_center(s, i, i);
        if (odd_end - odd_start > best_end - best_start) {
            best_start = odd_start;
            best_end = odd_end;
        }

        const auto [even_start, even_end] = expand_from_center(s, i, i + 1);
        if (even_end - even_start > best_end - best_start) {
            best_start = even_start;
            best_end = even_end;
        }
    }
    return std::string{s.substr(static_cast<std::size_t>(best_start),
                                static_cast<std::size_t>(best_end - best_start + 1))};
}

/// Count the total number of palindromic substrings in the input.
inline int palindromic_substring_count(std::string_view s) {
    int total = 0;
    for (int center = 0; center < static_cast<int>(s.size()); ++center) {
        int left = center;
        int right = center;
        while (left >= 0 && right < static_cast<int>(s.size()) &&
               s[static_cast<std::size_t>(left)] ==
                   s[static_cast<std::size_t>(right)]) {
            ++total;
            --left;
            ++right;
        }
        left = center;
        right = center + 1;
        while (left >= 0 && right < static_cast<int>(s.size()) &&
               s[static_cast<std::size_t>(left)] ==
                   s[static_cast<std::size_t>(right)]) {
            ++total;
            --left;
            ++right;
        }
    }
    return total;
}

/// Count the number of ways to decode a numeric string mapping to letters.
inline int decode_ways(std::string_view s) {
    if (s.empty() || s.front() == '0')
        return 0;

    int prev2 = 1; // ways to decode up to i-2
    int prev1 = 1; // ways to decode up to i-1
    for (std::size_t i = 1; i < s.size(); ++i) {
        int current = 0;
        if (s[i] != '0')
            current += prev1;
        const int two_digit = (s[i - 1] - '0') * 10 + (s[i] - '0');
        if (two_digit >= 10 && two_digit <= 26)
            current += prev2;
        if (current == 0)
            return 0;
        prev2 = prev1;
        prev1 = current;
    }
    return prev1;
}

/// Minimum number of coins required to compose the given amount.
inline int coin_change(const std::vector<int>& coins, int amount) {
    if (amount < 0)
        return -1;
    std::vector<int> dp(static_cast<std::size_t>(amount) + 1, amount + 1);
    dp[0] = 0;
    for (int coin : coins) {
        if (coin <= 0)
            continue;
        for (int value = coin; value <= amount; ++value) {
            const int prev = dp[static_cast<std::size_t>(value - coin)];
            if (prev != amount + 1)
                dp[static_cast<std::size_t>(value)] =
                    std::min(dp[static_cast<std::size_t>(value)], prev + 1);
        }
    }
    const int result = dp[static_cast<std::size_t>(amount)];
    return result > amount ? -1 : result;
}

/// Maximum product obtainable from a contiguous subarray.
inline int maximum_product_subarray(const std::vector<int>& nums) {
    if (nums.empty())
        return 0;
    int best = nums.front();
    int max_ending_here = nums.front();
    int min_ending_here = nums.front();
    for (std::size_t i = 1; i < nums.size(); ++i) {
        if (nums[i] < 0)
            std::swap(max_ending_here, min_ending_here);
        max_ending_here = std::max(nums[i], max_ending_here * nums[i]);
        min_ending_here = std::min(nums[i], min_ending_here * nums[i]);
        best = std::max(best, max_ending_here);
    }
    return best;
}

/// Determine whether the input string can be segmented into dictionary words.
inline bool word_break(std::string_view s,
                       const std::vector<std::string>& word_dict) {
    std::unordered_set<std::string> dict(word_dict.begin(), word_dict.end());
    std::vector<bool> dp(s.size() + 1, false);
    dp[0] = true;
    for (std::size_t end = 1; end <= s.size(); ++end) {
        for (std::size_t start = 0; start < end; ++start) {
            if (dp[start] &&
                dict.find(std::string{s.substr(start, end - start)}) !=
                    dict.end()) {
                dp[end] = true;
                break;
            }
        }
    }
    return dp[s.size()];
}

/// Encode the deterministic word break result as a quantum-inspired probability.
inline qpp::pbool quantum_word_break(
    std::string_view s, const std::vector<std::string>& word_dict) {
    return qpp::pbool(word_break(s, word_dict) ? 1.0 : 0.0);
}

/// Length of the longest increasing subsequence using patience sorting.
inline int longest_increasing_subsequence(const std::vector<int>& nums) {
    std::vector<int> tails;
    tails.reserve(nums.size());
    for (int value : nums) {
        auto it = std::lower_bound(tails.begin(), tails.end(), value);
        if (it == tails.end())
            tails.push_back(value);
        else
            *it = value;
    }
    return static_cast<int>(tails.size());
}

} // namespace qpp::examples::dynamic_programming_1d

