#pragma once

#include <cstddef>
#include <utility>
#include <vector>

namespace qpp::graphs {

struct Node {
  int value{};
  std::vector<Node*> neighbors{};
};

std::size_t count_islands(const std::vector<std::vector<char>>& grid);

Node* clone_graph(Node* node);

std::vector<std::pair<int, int>> pacific_atlantic(
    const std::vector<std::vector<int>>& heights);

bool can_finish_courses(
    int num_courses,
    const std::vector<std::pair<int, int>>& prerequisites);

bool is_valid_tree(int n, const std::vector<std::pair<int, int>>& edges);

int count_components(int n, const std::vector<std::pair<int, int>>& edges);

}  // namespace qpp::graphs

