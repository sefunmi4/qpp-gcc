#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

#include "qpp/entangled_set"
#include "qpp/pbool.h"
#include "qpp/qstruct.hpp"
#include "qpp/qvector"

namespace qpp::examples::backtracking {
namespace detail {

inline std::size_t encode_position(std::size_t row, std::size_t col,
                                   std::size_t cols) noexcept {
    return row * cols + col;
}

inline void combination_sum_dfs(const qpp::qvector<int>& candidates,
                                int target, std::size_t index,
                                qpp::qvector<int>& combination,
                                qpp::qvector<qpp::qvector<int>>& result) {
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

inline bool word_search_dfs(const qpp::qvector<qpp::qvector<char>>& board,
                            std::string_view word, std::size_t depth, int row,
                            int col, std::entangled_set<std::size_t>& visited,
                            std::size_t cols) {
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
        if (board[r][c] != word[depth])
            continue;

        const std::size_t key = encode_position(r, c, cols);
        if (!visited.insert(key).second)
            continue;

        if (word_search_dfs(board, word, depth + 1, next_row, next_col, visited,
                            cols))
            return true;
        visited.erase(key);
    }

    return false;
}

} // namespace detail

/// Find all unique combinations of candidate numbers that sum to the target.
///
/// The candidates are deduplicated and sorted internally. Each value can be
/// used unlimited times. The algorithm performs a depth-first search while
/// pruning branches whenever the partial sum would exceed the target.
[[nodiscard]] inline qpp::qvector<qpp::qvector<int>>
combination_sum(qpp::qvector<int> candidates, int target) {
    if (target < 0)
        return {};

    std::entangled_set<int> unique_candidates;
    unique_candidates.reserve(candidates.size());
    for (int value : candidates)
        unique_candidates.insert(value);

    candidates.assign(unique_candidates.begin(), unique_candidates.end());
    std::sort(candidates.begin(), candidates.end());

    qpp::qvector<qpp::qvector<int>> result;
    qpp::qvector<int> combination;
    detail::combination_sum_dfs(candidates, target, 0, combination, result);
    return result;
}

/// Search a 2D character grid for a word by moving horizontally or vertically.
///
/// Returns `true` when the word can be formed without revisiting cells. The
/// recursion explores neighbours starting from every matching cell on the grid.
[[nodiscard]] inline bool word_search(
    const qpp::qvector<qpp::qvector<char>>& board, std::string_view word) {
    if (word.empty())
        return true;
    if (board.empty() || board.front().empty())
        return false;

    const std::size_t rows = board.size();
    const std::size_t cols = board.front().size();

    std::entangled_set<std::size_t> visited;
    visited.reserve(rows * cols);

    for (std::size_t r = 0; r < rows; ++r) {
        for (std::size_t c = 0; c < cols; ++c) {
            if (board[r][c] != word.front())
                continue;
            visited.clear();
            const std::size_t key = detail::encode_position(r, c, cols);
            visited.insert(key);
            if (detail::word_search_dfs(board, word, 1,
                                        static_cast<int>(r),
                                        static_cast<int>(c), visited, cols))
                return true;
            visited.erase(key);
        }
    }

    return false;
}

/// Convert a boolean existence result into a probabilistic boolean value. This
/// is primarily useful when integrating classical search results with quantum
/// control logic inside QPP examples.
[[nodiscard]] inline qpp::pbool word_search_probability(
    const qpp::qvector<qpp::qvector<char>>& board, std::string_view word) {
    qpp::qclass indicator(1);
    if (word_search(board, word))
        indicator.apply_x(0);
    return indicator.data().measure(0);
}

} // namespace qpp::examples::backtracking

