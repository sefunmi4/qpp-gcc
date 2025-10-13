#include "graph_examples.hpp"

#include <queue>
#include <unordered_map>

namespace qpp::graphs {

namespace {
constexpr int directions[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

void dfs_islands(const std::vector<std::vector<char>>& grid,
                 std::vector<std::vector<bool>>& visited, int r, int c) {
  const int rows = static_cast<int>(grid.size());
  const int cols = static_cast<int>(grid.front().size());
  visited[r][c] = true;
  for (const auto& dir : directions) {
    const int nr = r + dir[0];
    const int nc = c + dir[1];
    if (nr >= 0 && nr < rows && nc >= 0 && nc < cols &&
        !visited[nr][nc] && grid[nr][nc] == '1') {
      dfs_islands(grid, visited, nr, nc);
    }
  }
}

void dfs_clone(Node* node, std::unordered_map<Node*, Node*>& clones) {
  auto* clone = clones[node];
  for (Node* neighbor : node->neighbors) {
    if (!clones.count(neighbor)) {
      clones[neighbor] = new Node{neighbor->value, {}};
      dfs_clone(neighbor, clones);
    }
    clone->neighbors.push_back(clones[neighbor]);
  }
}

void dfs_flow(int r, int c, const std::vector<std::vector<int>>& heights,
              std::vector<std::vector<bool>>& reachable) {
  if (reachable[r][c]) {
    return;
  }
  reachable[r][c] = true;
  const int rows = static_cast<int>(heights.size());
  const int cols = static_cast<int>(heights.front().size());
  for (const auto& dir : directions) {
    const int nr = r + dir[0];
    const int nc = c + dir[1];
    if (nr >= 0 && nr < rows && nc >= 0 && nc < cols &&
        heights[nr][nc] >= heights[r][c]) {
      dfs_flow(nr, nc, heights, reachable);
    }
  }
}

}  // namespace

std::size_t count_islands(const std::vector<std::vector<char>>& grid) {
  if (grid.empty() || grid.front().empty()) {
    return 0;
  }

  const int rows = static_cast<int>(grid.size());
  const int cols = static_cast<int>(grid.front().size());
  std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
  std::size_t islands = 0;

  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < cols; ++c) {
      if (grid[r][c] == '1' && !visited[r][c]) {
        ++islands;
        dfs_islands(grid, visited, r, c);
      }
    }
  }
  return islands;
}

Node* clone_graph(Node* node) {
  if (!node) {
    return nullptr;
  }
  std::unordered_map<Node*, Node*> clones;
  clones[node] = new Node{node->value, {}};
  dfs_clone(node, clones);
  return clones[node];
}

std::vector<std::pair<int, int>> pacific_atlantic(
    const std::vector<std::vector<int>>& heights) {
  if (heights.empty() || heights.front().empty()) {
    return {};
  }

  const int rows = static_cast<int>(heights.size());
  const int cols = static_cast<int>(heights.front().size());
  std::vector<std::vector<bool>> pacific(rows, std::vector<bool>(cols, false));
  std::vector<std::vector<bool>> atlantic(rows, std::vector<bool>(cols, false));

  for (int c = 0; c < cols; ++c) {
    dfs_flow(0, c, heights, pacific);
    dfs_flow(rows - 1, c, heights, atlantic);
  }

  for (int r = 0; r < rows; ++r) {
    dfs_flow(r, 0, heights, pacific);
    dfs_flow(r, cols - 1, heights, atlantic);
  }

  std::vector<std::pair<int, int>> result;
  result.reserve(static_cast<std::size_t>(rows * cols));
  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < cols; ++c) {
      if (pacific[r][c] && atlantic[r][c]) {
        result.emplace_back(r, c);
      }
    }
  }
  return result;
}

bool can_finish_courses(
    int num_courses,
    const std::vector<std::pair<int, int>>& prerequisites) {
  std::vector<std::vector<int>> graph(num_courses);
  std::vector<int> indegree(num_courses, 0);
  for (const auto& [course, prereq] : prerequisites) {
    if (course < 0 || course >= num_courses || prereq < 0 ||
        prereq >= num_courses) {
      continue;
    }
    graph[prereq].push_back(course);
    ++indegree[course];
  }

  std::queue<int> queue;
  for (int i = 0; i < num_courses; ++i) {
    if (indegree[i] == 0) {
      queue.push(i);
    }
  }

  int visited = 0;
  while (!queue.empty()) {
    int course = queue.front();
    queue.pop();
    ++visited;
    for (int next : graph[course]) {
      if (--indegree[next] == 0) {
        queue.push(next);
      }
    }
  }
  return visited == num_courses;
}

bool is_valid_tree(int n, const std::vector<std::pair<int, int>>& edges) {
  if (edges.size() != static_cast<std::size_t>(n - 1)) {
    return false;
  }

  std::vector<std::vector<int>> graph(n);
  for (const auto& [u, v] : edges) {
    if (u < 0 || u >= n || v < 0 || v >= n) {
      return false;
    }
    graph[u].push_back(v);
    graph[v].push_back(u);
  }

  std::vector<bool> visited(n, false);
  std::queue<int> queue;
  queue.push(0);
  visited[0] = true;

  int count = 0;
  while (!queue.empty()) {
    int node = queue.front();
    queue.pop();
    ++count;
    for (int neighbor : graph[node]) {
      if (!visited[neighbor]) {
        visited[neighbor] = true;
        queue.push(neighbor);
      }
    }
  }
  return count == n;
}

int count_components(int n, const std::vector<std::pair<int, int>>& edges) {
  std::vector<std::vector<int>> graph(n);
  for (const auto& [u, v] : edges) {
    if (u < 0 || u >= n || v < 0 || v >= n) {
      continue;
    }
    graph[u].push_back(v);
    graph[v].push_back(u);
  }

  std::vector<bool> visited(n, false);
  int components = 0;
  std::queue<int> queue;

  for (int i = 0; i < n; ++i) {
    if (visited[i]) {
      continue;
    }
    ++components;
    visited[i] = true;
    queue.push(i);

    while (!queue.empty()) {
      int node = queue.front();
      queue.pop();
      for (int neighbor : graph[node]) {
        if (!visited[neighbor]) {
          visited[neighbor] = true;
          queue.push(neighbor);
        }
      }
    }
  }
  return components;
}

}  // namespace qpp::graphs

