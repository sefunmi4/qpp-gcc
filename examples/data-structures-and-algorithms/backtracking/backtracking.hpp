#pragma once

#include <algorithm>
#include <array>
#include <string>
#include <string_view>
#include <vector>

#include "qpp/pbool.h"

namespace qpp::examples::backtracking {
namespace detail {

inline void combination_sum_dfs(const std::vector<int>& candidates,
                                int target, std::size_t index,
                                std::vector<int>& combination,
                                std::vector<std::vector<int>>& result) {
    if (target == 0) {
        result.push_back(combination);
        return;
    }

    for (std::size_t i = index; i < candidates.size(); ++i) {
        const int value = candidates[i];
        if (value > target)
            break; // All larger values will also be too big.

        combination.push_back(value);
        combination_sum_dfs(candidates, target - value, i, combination, result);
        combination.pop_back();
    }
}

inline bool word_search_dfs(const std::vector<std::vector<char>>& board,
                            std::string_view word, std::size_t depth, int row,
                            int col, std::vector<std::vector<bool>>& visited) {
    if (depth == word.size())
        return true;

    constexpr std::array<std::pair<int, int>, 4> directions{
        {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};

    for (const auto [dr, dc] : directions) {
        const int next_row = row + dr;
        const int next_col = col + dc;
        if (next_row < 0 || next_col < 0)
            continue;
        const std::size_t r = static_cast<std::size_t>(next_row);
        const std::size_t c = static_cast<std::size_t>(next_col);
        if (r >= board.size() || c >= board[r].size())
            continue;
        if (visited[r][c] || board[r][c] != word[depth])
            continue;

        visited[r][c] = true;
        if (word_search_dfs(board, word, depth + 1, next_row, next_col,
                            visited))
            return true;
        visited[r][c] = false;
    }

    return false;
}

} // namespace detail

/// Find all unique combinations of candidate numbers that sum to the target.
///
/// The candidates are deduplicated and sorted internally. Each value can be
/// used unlimited times. The algorithm performs a depth-first search while
/// pruning branches whenever the partial sum would exceed the target.
[[nodiscard]] inline std::vector<std::vector<int>>
combination_sum(std::vector<int> candidates, int target) {
    if (target < 0)
        return {};

    std::sort(candidates.begin(), candidates.end());
    candidates.erase(std::unique(candidates.begin(), candidates.end()),
                     candidates.end());

    std::vector<std::vector<int>> result;
    std::vector<int> combination;
    detail::combination_sum_dfs(candidates, target, 0, combination, result);
    return result;
}

/// Search a 2D character grid for a word by moving horizontally or vertically.
///
/// Returns `true` when the word can be formed without revisiting cells. The
/// recursion explores neighbours starting from every matching cell on the grid.
[[nodiscard]] inline bool word_search(
    const std::vector<std::vector<char>>& board, std::string_view word) {
    if (word.empty())
        return true;
    if (board.empty() || board.front().empty())
        return false;

    const std::size_t rows = board.size();
    const std::size_t cols = board.front().size();

    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));

    for (std::size_t r = 0; r < rows; ++r) {
        for (std::size_t c = 0; c < cols; ++c) {
            if (board[r][c] != word.front())
                continue;
            visited[r][c] = true;
            if (detail::word_search_dfs(board, word, 1,
                                        static_cast<int>(r),
                                        static_cast<int>(c), visited))
                return true;
            visited[r][c] = false;
        }
    }

    return false;
}

/// Convert a boolean existence result into a probabilistic boolean value. This
/// is primarily useful when integrating classical search results with quantum
/// control logic inside QPP examples.
[[nodiscard]] inline qpp::pbool word_search_probability(
    const std::vector<std::vector<char>>& board, std::string_view word) {
    return qpp::pbool(word_search(board, word) ? 1.0 : 0.0);
}

} // namespace qpp::examples::backtracking

