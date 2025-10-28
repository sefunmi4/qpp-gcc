#pragma once

#include <cstddef>
#include <functional>
#include <utility>

#include "qpp/entangled_map"
#include "qpp/entangled_set"
#include "qpp/qvector"

namespace qpp::examples::graph {

struct Node {
    int value{};
    qpp::qvector<Node*> neighbors{};
};

using NodeCloneMap =
    std::entangled_map<Node*, Node*, std::hash<Node*>, std::equal_to<Node*>>;

namespace detail {
inline constexpr int kDirections[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

inline void dfs_islands(const qpp::qvector<qpp::qvector<char>>& grid,
                        qpp::qvector<qpp::qvector<bool>>& visited, int row,
                        int col) {
    const int rows = static_cast<int>(grid.size());
    const int cols = static_cast<int>(grid.front().size());

    visited[row][col] = true;
    for (const auto& direction : kDirections) {
        const int next_row = row + direction[0];
        const int next_col = col + direction[1];
        if (next_row >= 0 && next_row < rows && next_col >= 0 &&
            next_col < cols && !visited[next_row][next_col] &&
            grid[next_row][next_col] == '1') {
            dfs_islands(grid, visited, next_row, next_col);
        }
    }
}

inline void dfs_clone(Node* node, NodeCloneMap& clones) {
    auto* clone = clones[node];
    for (Node* neighbor : node->neighbors) {
        if (!clones.count(neighbor)) {
            clones[neighbor] = new Node{neighbor->value, {}};
            dfs_clone(neighbor, clones);
        }
        clone->neighbors.push_back(clones[neighbor]);
    }
}

inline void dfs_flow(int row, int col,
                     const qpp::qvector<qpp::qvector<int>>& heights,
                     qpp::qvector<qpp::qvector<bool>>& reachable) {
    if (reachable[row][col])
        return;

    reachable[row][col] = true;
    const int rows = static_cast<int>(heights.size());
    const int cols = static_cast<int>(heights.front().size());
    for (const auto& direction : kDirections) {
        const int next_row = row + direction[0];
        const int next_col = col + direction[1];
        if (next_row >= 0 && next_row < rows && next_col >= 0 &&
            next_col < cols && heights[next_row][next_col] >=
                                  heights[row][col]) {
            dfs_flow(next_row, next_col, heights, reachable);
        }
    }
}

} // namespace detail

inline std::size_t count_islands(
    const qpp::qvector<qpp::qvector<char>>& grid) {
    if (grid.empty() || grid.front().empty())
        return 0;

    const int rows = static_cast<int>(grid.size());
    const int cols = static_cast<int>(grid.front().size());
    qpp::qvector<qpp::qvector<bool>> visited(static_cast<std::size_t>(rows),
                                            qpp::qvector<bool>(static_cast<std::size_t>(cols)));
    std::size_t islands = 0;

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            if (grid[row][col] == '1' && !visited[row][col]) {
                ++islands;
                detail::dfs_islands(grid, visited, row, col);
            }
        }
    }

    return islands;
}

inline Node* clone_graph(Node* node) {
    if (!node)
        return nullptr;

    NodeCloneMap clones;
    clones[node] = new Node{node->value, {}};
    detail::dfs_clone(node, clones);
    return clones[node];
}

inline qpp::qvector<std::pair<int, int>> pacific_atlantic(
    const qpp::qvector<qpp::qvector<int>>& heights) {
    if (heights.empty() || heights.front().empty())
        return {};

    const int rows = static_cast<int>(heights.size());
    const int cols = static_cast<int>(heights.front().size());
    qpp::qvector<qpp::qvector<bool>> pacific(static_cast<std::size_t>(rows),
                                             qpp::qvector<bool>(static_cast<std::size_t>(cols)));
    qpp::qvector<qpp::qvector<bool>> atlantic(static_cast<std::size_t>(rows),
                                              qpp::qvector<bool>(static_cast<std::size_t>(cols)));

    for (int col = 0; col < cols; ++col) {
        detail::dfs_flow(0, col, heights, pacific);
        detail::dfs_flow(rows - 1, col, heights, atlantic);
    }

    for (int row = 0; row < rows; ++row) {
        detail::dfs_flow(row, 0, heights, pacific);
        detail::dfs_flow(row, cols - 1, heights, atlantic);
    }

    qpp::qvector<std::pair<int, int>> result;
    result.reserve(static_cast<std::size_t>(rows * cols));
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            if (pacific[row][col] && atlantic[row][col])
                result.emplace_back(row, col);
        }
    }

    return result;
}

inline bool can_finish_courses(
    int num_courses, const qpp::qvector<std::pair<int, int>>& prerequisites) {
    if (num_courses <= 0)
        return prerequisites.empty();

    qpp::qvector<qpp::qvector<int>> graph(static_cast<std::size_t>(num_courses));
    qpp::qvector<int> indegree(static_cast<std::size_t>(num_courses), 0);
    for (const auto& [course, prereq] : prerequisites) {
        if (course < 0 || course >= num_courses || prereq < 0 ||
            prereq >= num_courses)
            continue;

        graph[prereq].push_back(course);
        ++indegree[course];
    }

    qpp::qvector<int> queue;
    queue.reserve(static_cast<std::size_t>(num_courses));
    for (int course = 0; course < num_courses; ++course) {
        if (indegree[static_cast<std::size_t>(course)] == 0)
            queue.push_back(course);
    }

    std::size_t head = 0;
    int visited = 0;
    while (head < queue.size()) {
        const int course = queue[head++];
        ++visited;

        for (int next : graph[static_cast<std::size_t>(course)]) {
            auto& indegree_ref = indegree[static_cast<std::size_t>(next)];
            if (--indegree_ref == 0)
                queue.push_back(next);
        }
    }

    return visited == num_courses;
}

inline bool is_valid_tree(int n,
                          const qpp::qvector<std::pair<int, int>>& edges) {
    if (n <= 0)
        return false;

    if (edges.size() != static_cast<std::size_t>(n - 1))
        return false;

    qpp::qvector<qpp::qvector<int>> graph(static_cast<std::size_t>(n));
    for (const auto& [u, v] : edges) {
        if (u < 0 || u >= n || v < 0 || v >= n)
            return false;

        graph[u].push_back(v);
        graph[v].push_back(u);
    }

    qpp::qvector<bool> visited(static_cast<std::size_t>(n), false);
    qpp::qvector<int> queue;
    queue.push_back(0);
    visited[0] = true;

    int count = 0;
    std::size_t head = 0;
    while (head < queue.size()) {
        const int node = queue[head++];
        ++count;

        for (int neighbor : graph[static_cast<std::size_t>(node)]) {
            if (!visited[static_cast<std::size_t>(neighbor)]) {
                visited[static_cast<std::size_t>(neighbor)] = true;
                queue.push_back(neighbor);
            }
        }
    }

    return count == n;
}

inline int count_components(
    int n, const qpp::qvector<std::pair<int, int>>& edges) {
    if (n <= 0)
        return 0;

    qpp::qvector<qpp::qvector<int>> graph(static_cast<std::size_t>(n));
    for (const auto& [u, v] : edges) {
        if (u < 0 || u >= n || v < 0 || v >= n)
            continue;

        graph[u].push_back(v);
        graph[v].push_back(u);
    }

    qpp::qvector<bool> visited(static_cast<std::size_t>(n), false);
    qpp::qvector<int> queue;
    int components = 0;

    for (int node = 0; node < n; ++node) {
        if (visited[node])
            continue;

        ++components;
        visited[static_cast<std::size_t>(node)] = true;
        queue.clear();
        queue.push_back(node);
        std::size_t head = 0;

        while (head < queue.size()) {
            const int current = queue[head++];

            for (int neighbor : graph[static_cast<std::size_t>(current)]) {
                if (!visited[static_cast<std::size_t>(neighbor)]) {
                    visited[static_cast<std::size_t>(neighbor)] = true;
                    queue.push_back(neighbor);
                }
            }
        }
    }

    return components;
}

} // namespace qpp::examples::graph

