#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace qpp::examples::dynamic_programming_2d {

/// Count the number of unique paths from the top-left to the bottom-right of an
/// m x n grid when you may only move right or down.
inline int unique_paths(int m, int n) {
    if (m <= 0 || n <= 0)
        return 0;
    std::vector<int> dp(static_cast<std::size_t>(n), 1);
    for (int row = 1; row < m; ++row) {
        for (int col = 1; col < n; ++col)
            dp[static_cast<std::size_t>(col)] +=
                dp[static_cast<std::size_t>(col - 1)];
    }
    return dp.back();
}

/// Compute the longest common subsequence shared by two input strings using a
/// classic dynamic programming table.
inline std::string longest_common_subsequence(std::string_view first,
                                              std::string_view second) {
    const std::size_t rows = first.size();
    const std::size_t cols = second.size();
    std::vector<std::vector<int>> dp(rows + 1, std::vector<int>(cols + 1, 0));

    for (std::size_t i = 1; i <= rows; ++i) {
        for (std::size_t j = 1; j <= cols; ++j) {
            if (first[i - 1] == second[j - 1])
                dp[i][j] = dp[i - 1][j - 1] + 1;
            else
                dp[i][j] = std::max(dp[i - 1][j], dp[i][j - 1]);
        }
    }

    std::string result;
    result.reserve(static_cast<std::size_t>(dp[rows][cols]));
    std::size_t i = rows;
    std::size_t j = cols;
    while (i > 0 && j > 0) {
        if (first[i - 1] == second[j - 1]) {
            result.push_back(first[i - 1]);
            --i;
            --j;
        } else if (dp[i - 1][j] >= dp[i][j - 1]) {
            --i;
        } else {
            --j;
        }
    }
    std::reverse(result.begin(), result.end());
    return result;
}

} // namespace qpp::examples::dynamic_programming_2d

