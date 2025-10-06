#pragma once

#include <algorithm>
#include <limits>
#include <memory>
#include <optional>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace qpp::examples::tree {

struct TreeNode {
    int value{};
    std::shared_ptr<TreeNode> left{};
    std::shared_ptr<TreeNode> right{};
};

using NodePtr = std::shared_ptr<TreeNode>;

inline NodePtr make_node(int value, NodePtr left = nullptr, NodePtr right = nullptr) {
    return std::make_shared<TreeNode>(TreeNode{value, std::move(left), std::move(right)});
}

inline NodePtr make_tree(const std::vector<std::optional<int>>& values) {
    if (values.empty() || !values.front())
        return nullptr;

    std::vector<NodePtr> nodes(values.size());
    nodes.front() = make_node(*values.front());

    for (std::size_t index = 0; index < values.size(); ++index) {
        const auto& value = values[index];
        if (!value)
            continue;

        auto& node = nodes[index];
        if (!node)
            node = make_node(*value);

        const auto left_index = 2 * index + 1;
        const auto right_index = 2 * index + 2;

        if (left_index < values.size() && values[left_index]) {
            nodes[left_index] = make_node(*values[left_index]);
            node->left = nodes[left_index];
        }

        if (right_index < values.size() && values[right_index]) {
            nodes[right_index] = make_node(*values[right_index]);
            node->right = nodes[right_index];
        }
    }

    return nodes.front();
}

inline std::string serialize(const NodePtr& root) {
    if (!root)
        return "null";

    std::ostringstream stream;
    std::queue<NodePtr> queue;
    queue.push(root);

    bool first = true;
    while (!queue.empty()) {
        const auto node = queue.front();
        queue.pop();

        if (!first)
            stream << ',';
        first = false;

        if (node) {
            stream << node->value;
            queue.push(node->left);
            queue.push(node->right);
        } else {
            stream << '#';
        }
    }

    return stream.str();
}

inline NodePtr deserialize(const std::string& data) {
    if (data == "null" || data.empty())
        return nullptr;

    std::vector<std::optional<int>> values;
    std::size_t start = 0;
    while (start <= data.size()) {
        const auto end = data.find(',', start);
        const auto token = data.substr(start, end == std::string::npos ? std::string::npos : end - start);
        if (token == "#" || token == "null") {
            values.push_back(std::nullopt);
        } else {
            values.push_back(std::stoi(token));
        }

        if (end == std::string::npos)
            break;
        start = end + 1;
    }

    if (values.empty() || !values.front())
        return nullptr;

    NodePtr root = make_node(*values.front());
    std::queue<NodePtr> queue;
    queue.push(root);

    std::size_t index = 1;
    while (!queue.empty() && index < values.size()) {
        const auto current = queue.front();
        queue.pop();

        if (values[index]) {
            current->left = make_node(*values[index]);
            queue.push(current->left);
        }
        ++index;
        if (index >= values.size())
            break;

        if (values[index]) {
            current->right = make_node(*values[index]);
            queue.push(current->right);
        }
        ++index;
    }

    return root;
}

inline NodePtr invert_tree(NodePtr root) {
    if (!root)
        return nullptr;

    std::swap(root->left, root->right);
    invert_tree(root->left);
    invert_tree(root->right);
    return root;
}

inline int max_depth(const NodePtr& root) {
    if (!root)
        return 0;

    return 1 + std::max(max_depth(root->left), max_depth(root->right));
}

inline bool same_tree(const NodePtr& left, const NodePtr& right) {
    if (!left || !right)
        return !left && !right;

    return left->value == right->value && same_tree(left->left, right->left) &&
           same_tree(left->right, right->right);
}

inline bool is_subtree(const NodePtr& root, const NodePtr& sub) {
    if (!sub)
        return true;
    if (!root)
        return false;

    if (same_tree(root, sub))
        return true;

    return is_subtree(root->left, sub) || is_subtree(root->right, sub);
}

inline NodePtr lowest_common_ancestor_bst(NodePtr root, int value1, int value2) {
    while (root) {
        if (value1 < root->value && value2 < root->value) {
            root = root->left;
        } else if (value1 > root->value && value2 > root->value) {
            root = root->right;
        } else {
            return root;
        }
    }

    return nullptr;
}

inline std::vector<std::vector<int>> level_order(const NodePtr& root) {
    std::vector<std::vector<int>> levels;
    if (!root)
        return levels;

    std::queue<NodePtr> queue;
    queue.push(root);

    while (!queue.empty()) {
        const auto size = queue.size();
        std::vector<int> level;
        level.reserve(size);

        for (std::size_t index = 0; index < size; ++index) {
            const auto node = queue.front();
            queue.pop();
            level.push_back(node->value);

            if (node->left)
                queue.push(node->left);
            if (node->right)
                queue.push(node->right);
        }

        levels.push_back(std::move(level));
    }

    return levels;
}

inline bool is_valid_bst(const NodePtr& root, std::optional<int> min = std::nullopt,
                         std::optional<int> max = std::nullopt) {
    if (!root)
        return true;

    if ((min && root->value <= *min) || (max && root->value >= *max))
        return false;

    return is_valid_bst(root->left, min, root->value) &&
           is_valid_bst(root->right, root->value, max);
}

inline std::optional<int> kth_smallest(const NodePtr& root, int k) {
    std::vector<NodePtr> stack;
    auto current = root;
    int count = 0;

    while (current || !stack.empty()) {
        while (current) {
            stack.push_back(current);
            current = current->left;
        }

        current = stack.back();
        stack.pop_back();
        ++count;
        if (count == k)
            return current->value;

        current = current->right;
    }

    return std::nullopt;
}

inline NodePtr build_tree_impl(const std::vector<int>& preorder, std::size_t pre_left,
                               std::size_t pre_right,
                               const std::unordered_map<int, std::size_t>& inorder_index,
                               std::size_t in_left) {
    if (pre_left > pre_right)
        return nullptr;

    const int root_value = preorder[pre_left];
    const auto root = make_node(root_value);

    const auto inorder_position = inorder_index.at(root_value);
    const auto left_size = inorder_position - in_left;

    root->left = build_tree_impl(preorder, pre_left + 1, pre_left + left_size, inorder_index,
                                 in_left);
    root->right = build_tree_impl(preorder, pre_left + left_size + 1, pre_right, inorder_index,
                                  inorder_position + 1);

    return root;
}

inline NodePtr build_tree(const std::vector<int>& preorder, const std::vector<int>& inorder) {
    if (preorder.size() != inorder.size() || preorder.empty())
        return nullptr;

    std::unordered_map<int, std::size_t> inorder_index;
    inorder_index.reserve(inorder.size());
    for (std::size_t index = 0; index < inorder.size(); ++index)
        inorder_index[inorder[index]] = index;

    return build_tree_impl(preorder, 0, preorder.size() - 1, inorder_index, 0);
}

inline int max_path_sum_impl(const NodePtr& node, int& best) {
    if (!node)
        return 0;

    const int left_gain = std::max(0, max_path_sum_impl(node->left, best));
    const int right_gain = std::max(0, max_path_sum_impl(node->right, best));

    best = std::max(best, node->value + left_gain + right_gain);
    return node->value + std::max(left_gain, right_gain);
}

inline int max_path_sum(const NodePtr& root) {
    int best = std::numeric_limits<int>::min();
    max_path_sum_impl(root, best);
    return best;
}

inline std::string describe_levels(const std::vector<std::vector<int>>& levels) {
    std::ostringstream stream;
    stream << '[';
    for (std::size_t level_index = 0; level_index < levels.size(); ++level_index) {
        if (level_index > 0)
            stream << ", ";
        stream << '[';
        for (std::size_t value_index = 0; value_index < levels[level_index].size();
             ++value_index) {
            if (value_index > 0)
                stream << ", ";
            stream << levels[level_index][value_index];
        }
        stream << ']';
    }
    stream << ']';
    return stream.str();
}

} // namespace qpp::examples::tree

