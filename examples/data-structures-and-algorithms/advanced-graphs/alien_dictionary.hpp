#pragma once

#include <algorithm>
#include <cstddef>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace qpp::examples::advanced_graphs {

using Graph = std::unordered_map<char, std::unordered_set<char>>;
using Indegrees = std::unordered_map<char, std::size_t>;

struct GraphAnalysis {
    Graph adjacency;
    Indegrees indegree;
    bool prefix_conflict{false};
};

struct AlienDictionaryResult {
    std::string order;
    bool valid{true};
    bool has_cycle{false};
    bool has_prefix_conflict{false};
    bool is_unique{false};
    std::vector<std::vector<char>> strongly_connected;
};

inline GraphAnalysis build_dependency_graph(const std::vector<std::string>& words) {
    GraphAnalysis analysis{};
    auto& adjacency = analysis.adjacency;
    auto& indegree = analysis.indegree;

    for (const auto& word : words) {
        for (const auto ch : word) {
            adjacency.try_emplace(ch);
            indegree.try_emplace(ch, 0);
        }
    }

    for (std::size_t index = 0; index + 1 < words.size(); ++index) {
        const auto& current = words[index];
        const auto& next = words[index + 1];
        const auto mismatch = std::mismatch(current.begin(), current.end(), next.begin(), next.end());

        if (mismatch.first == current.end())
            continue;

        if (mismatch.second == next.end()) {
            analysis.prefix_conflict = true;
            continue;
        }

        const auto from = *mismatch.first;
        const auto to = *mismatch.second;
        auto& edges = adjacency[from];
        if (edges.insert(to).second) {
            ++indegree[to];
        }
    }

    return analysis;
}

inline void tarjan_dfs(const Graph& graph, char node, std::size_t& index,
                       std::unordered_map<char, std::size_t>& indices,
                       std::unordered_map<char, std::size_t>& lowlinks,
                       std::unordered_set<char>& on_stack, std::stack<char>& stack,
                       std::vector<std::vector<char>>& components) {
    indices[node] = index;
    lowlinks[node] = index;
    ++index;
    stack.push(node);
    on_stack.insert(node);

    if (const auto it = graph.find(node); it != graph.end()) {
        for (const auto neighbor : it->second) {
            if (!indices.count(neighbor)) {
                tarjan_dfs(graph, neighbor, index, indices, lowlinks, on_stack, stack, components);
                lowlinks[node] = std::min(lowlinks[node], lowlinks[neighbor]);
            } else if (on_stack.count(neighbor)) {
                lowlinks[node] = std::min(lowlinks[node], indices[neighbor]);
            }
        }
    }

    if (lowlinks[node] == indices[node]) {
        std::vector<char> component;
        while (!stack.empty()) {
            const auto top = stack.top();
            stack.pop();
            on_stack.erase(top);
            component.push_back(top);
            if (top == node)
                break;
        }
        components.push_back(std::move(component));
    }
}

inline std::vector<std::vector<char>> strongly_connected_components(const Graph& graph) {
    std::unordered_map<char, std::size_t> indices;
    std::unordered_map<char, std::size_t> lowlinks;
    std::unordered_set<char> on_stack;
    std::stack<char> stack;
    std::vector<std::vector<char>> components;
    std::size_t index = 0;

    for (const auto& [node, _] : graph) {
        if (!indices.count(node))
            tarjan_dfs(graph, node, index, indices, lowlinks, on_stack, stack, components);
    }

    for (auto& component : components)
        std::sort(component.begin(), component.end());

    std::sort(components.begin(), components.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.size() != rhs.size())
            return lhs.size() < rhs.size();
        return lhs < rhs;
    });

    return components;
}

inline AlienDictionaryResult alien_dictionary(const std::vector<std::string>& words) {
    AlienDictionaryResult result{};

    const auto analysis = build_dependency_graph(words);
    result.has_prefix_conflict = analysis.prefix_conflict;
    result.strongly_connected = strongly_connected_components(analysis.adjacency);

    auto indegree = analysis.indegree;
    std::vector<char> queue;
    for (const auto& [node, degree] : indegree) {
        if (degree == 0)
            queue.push_back(node);
    }

    std::size_t processed = 0;
    result.is_unique = true;

    while (!queue.empty()) {
        if (queue.size() > 1)
            result.is_unique = false;

        std::sort(queue.begin(), queue.end());
        const auto node = queue.front();
        queue.erase(queue.begin());
        result.order.push_back(node);
        ++processed;

        if (const auto it = analysis.adjacency.find(node); it != analysis.adjacency.end()) {
            for (const auto neighbor : it->second) {
                auto& degree = indegree[neighbor];
                if (degree > 0)
                    --degree;
                if (degree == 0)
                    queue.push_back(neighbor);
            }
        }
    }

    result.valid = processed == indegree.size() && !result.has_prefix_conflict;
    result.has_cycle = processed != indegree.size() || result.has_prefix_conflict;
    if (!result.valid)
        result.order.clear();

    return result;
}

inline std::string describe_components(const std::vector<std::vector<char>>& components) {
    std::string description;
    bool first_component = true;
    for (const auto& component : components) {
        if (!first_component)
            description += " | ";
        first_component = false;
        description.push_back('[');
        bool first = true;
        for (const auto node : component) {
            if (!first)
                description.push_back(',');
            first = false;
            description.push_back(node);
        }
        description.push_back(']');
    }
    return description;
}

} // namespace qpp::examples::advanced_graphs

