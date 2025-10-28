#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <string>
#include <string_view>

#include "qpp/pbool.h"
#include "qpp/qstruct.hpp"
#include "qpp/qvector"
#include "qpp/quantum_dp.hpp"

namespace qpp::examples::dynamic_programming_2d {

/// Count the number of unique paths from the top-left to the bottom-right of an
/// m x n grid when you may only move right or down. The classical count is
/// represented as a probabilistic qpp::pbool value to integrate with quantum
/// aware APIs.
inline qpp::pbool unique_paths(int m, int n) {
    if (m <= 0 || n <= 0)
        return qpp::pbool{0.0};

    qpp::qvector<int> dp(static_cast<std::size_t>(n), 1);
    for (int row = 1; row < m; ++row) {
        for (int col = 1; col < n; ++col)
            dp[static_cast<std::size_t>(col)] +=
                dp[static_cast<std::size_t>(col - 1)];
    }

    const int classical_paths = dp.back();
    const qpp::qdp::quantum_scalar<int> quantum_paths(classical_paths);

    const int steps = m + n - 2;
    const double total_sequences = steps <= 0 ? 1.0 : std::pow(2.0, steps);
    const double probability = total_sequences > 0.0
                                   ? static_cast<double>(quantum_paths.value()) /
                                         total_sequences
                                   : 0.0;

    return qpp::pbool{probability};
}

/// Compute the longest common subsequence shared by two input strings using a
/// classic dynamic programming table but surfaced as a qpp::qdp::quantum_string
/// so callers may inspect either the classical characters or the quantum
/// amplitudes.
inline qpp::qdp::quantum_string
longest_common_subsequence(std::string_view first, std::string_view second) {
    const std::size_t rows = first.size();
    const std::size_t cols = second.size();

    qpp::qvector<qpp::qdp::quantum_sequence<int>> dp;
    dp.reserve(rows + 1);
    for (std::size_t i = 0; i <= rows; ++i)
        dp.emplace_back(cols + 1, 0);

    for (std::size_t i = 1; i <= rows; ++i) {
        for (std::size_t j = 1; j <= cols; ++j) {
            if (first[i - 1] == second[j - 1])
                dp[i][j] = dp[i - 1][j - 1] + 1;
            else
                dp[i][j] = std::max(dp[i - 1][j], dp[i][j - 1]);
        }
        dp[i].refresh_state();
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

    qpp::qdp::quantum_string quantum_result(result);
    quantum_result.refresh_state();
    return quantum_result;
}

} // namespace qpp::examples::dynamic_programming_2d

