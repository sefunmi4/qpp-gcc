#pragma once

#include <algorithm>
#include <cstddef>

#include "qpp/qvector"

namespace qpp::examples::math_and_geometry {

/// Rotate an \c n x \c n matrix clockwise in-place.
///
/// The algorithm performs a layered rotation where every element of the
/// outer ring is swapped into its rotated position using temporary storage
/// for the four involved values. The process is then repeated for the next
/// inner ring until the center of the matrix is reached.
inline void rotate_image(std::qvector<std::qvector<int>>& matrix) {
    const std::size_t n = matrix.size();
    if (n == 0)
        return;

    for (std::size_t layer = 0; layer < n / 2; ++layer) {
        const std::size_t first = layer;
        const std::size_t last = n - 1 - layer;
        for (std::size_t i = first; i < last; ++i) {
            const std::size_t offset = i - first;
            const int top = matrix[first][i];

            // left -> top
            matrix[first][i] = matrix[last - offset][first];

            // bottom -> left
            matrix[last - offset][first] = matrix[last][last - offset];

            // right -> bottom
            matrix[last][last - offset] = matrix[i][last];

            // top -> right
            matrix[i][last] = top;
        }
    }
}

/// Read a matrix in spiral order (clockwise, starting from the top-left).
///
/// The traversal maintains shrinking boundaries for the rows and columns
/// and iteratively walks along the four directions (right, down, left,
/// up) until the full matrix has been consumed.
inline std::qvector<int>
spiral_matrix(const std::qvector<std::qvector<int>>& matrix) {
    if (matrix.empty() || matrix.front().empty())
        return {};

    std::qvector<int> result;
    const std::ptrdiff_t row_count =
        static_cast<std::ptrdiff_t>(matrix.size());
    const std::ptrdiff_t col_count =
        static_cast<std::ptrdiff_t>(matrix.front().size());

    std::ptrdiff_t top = 0;
    std::ptrdiff_t bottom = row_count - 1;
    std::ptrdiff_t left = 0;
    std::ptrdiff_t right = col_count - 1;

    while (top <= bottom && left <= right) {
        for (std::ptrdiff_t col = left; col <= right; ++col)
            result.push_back(
                matrix[static_cast<std::size_t>(top)][static_cast<std::size_t>(col)]);
        ++top;
        if (top > bottom)
            break;

        for (std::ptrdiff_t row = top; row <= bottom; ++row)
            result.push_back(matrix[static_cast<std::size_t>(row)]
                                       [static_cast<std::size_t>(right)]);
        --right;
        if (left > right)
            break;

        for (std::ptrdiff_t col = right; col >= left; --col)
            result.push_back(matrix[static_cast<std::size_t>(bottom)]
                                       [static_cast<std::size_t>(col)]);
        --bottom;
        if (top > bottom)
            break;

        for (std::ptrdiff_t row = bottom; row >= top; --row)
            result.push_back(matrix[static_cast<std::size_t>(row)]
                                       [static_cast<std::size_t>(left)]);
        ++left;
    }

    return result;
}

/// Set the entire row and column to zero for every zero element in the matrix.
///
/// The implementation uses the first row and column as marker storage to
/// achieve constant extra space while keeping track of which rows and
/// columns must be zeroed out.
inline void set_matrix_zeroes(std::qvector<std::qvector<int>>& matrix) {
    if (matrix.empty() || matrix.front().empty())
        return;

    const std::size_t rows = matrix.size();
    const std::size_t cols = matrix.front().size();

    bool first_row_zero = false;
    bool first_col_zero = false;

    for (std::size_t col = 0; col < cols; ++col) {
        if (matrix[0][col] == 0) {
            first_row_zero = true;
            break;
        }
    }

    for (std::size_t row = 0; row < rows; ++row) {
        if (matrix[row][0] == 0) {
            first_col_zero = true;
            break;
        }
    }

    for (std::size_t row = 1; row < rows; ++row) {
        for (std::size_t col = 1; col < cols; ++col) {
            if (matrix[row][col] == 0) {
                matrix[row][0] = 0;
                matrix[0][col] = 0;
            }
        }
    }

    for (std::size_t row = 1; row < rows; ++row) {
        if (matrix[row][0] == 0) {
            std::fill(matrix[row].begin(), matrix[row].end(), 0);
        }
    }

    for (std::size_t col = 1; col < cols; ++col) {
        if (matrix[0][col] == 0) {
            for (std::size_t row = 0; row < rows; ++row)
                matrix[row][col] = 0;
        }
    }

    if (first_row_zero) {
        std::fill(matrix[0].begin(), matrix[0].end(), 0);
    }

    if (first_col_zero) {
        for (std::size_t row = 0; row < rows; ++row)
            matrix[row][0] = 0;
    }
}

} // namespace qpp::examples::math_and_geometry

