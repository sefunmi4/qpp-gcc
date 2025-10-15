#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <string>

#include <qpp/entangled_map>
#include <qpp/entangled_set>
#include <qpp/qvector>

namespace qpp::examples::advanced_graphs {

using CharHash = std::hash<char>;
using CharEqual = std::equal_to<char>;
using CharSet = std::entangled_set<char, CharHash, CharEqual>;
using CharDegreeMap = std::entangled_map<char, std::size_t, CharHash, CharEqual>;
using Graph = std::entangled_map<char, CharSet, CharHash, CharEqual>;
using Indegrees = CharDegreeMap;
using CharStack = std::qvector<char>;
using CharQueue = std::qvector<char>;

struct GraphAnalysis {
    Graph adjacency;
    Indegrees indegree;
    bool prefix_conflict{false};
};

struct AlienDictionaryResult {
    std::qvector<char> order;
    bool valid{true};
    bool has_cycle{false};
    bool has_prefix_conflict{false};
    bool is_unique{false};
    std::qvector<std::qvector<char>> strongly_connected;
};

inline GraphAnalysis build_dependency_graph(const std::qvector<std::string>& words) {
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
                       CharDegreeMap& indices,
                       CharDegreeMap& lowlinks,
                       CharSet& on_stack, CharStack& stack,
                       std::qvector<std::qvector<char>>& components) {
    indices[node] = index;
    lowlinks[node] = index;
    ++index;
    stack.push_back(node);
    on_stack.insert(node);

    if (const auto it = graph.find(node); it != graph.end()) {
        std::qvector<char> neighbors(it->second.begin(), it->second.end());
        std::sort(neighbors.begin(), neighbors.end());
        for (const auto neighbor : neighbors) {
            if (!indices.count(neighbor)) {
                tarjan_dfs(graph, neighbor, index, indices, lowlinks, on_stack, stack, components);
                lowlinks[node] = std::min(lowlinks[node], lowlinks[neighbor]);
            } else if (on_stack.count(neighbor)) {
                lowlinks[node] = std::min(lowlinks[node], indices[neighbor]);
            }
        }
    }

    if (lowlinks[node] == indices[node]) {
        std::qvector<char> component;
        while (!stack.empty()) {
            const auto top = stack.back();
            stack.pop_back();
            on_stack.erase(top);
            component.push_back(top);
            if (top == node)
                break;
        }
        components.push_back(std::move(component));
    }
}

inline std::qvector<std::qvector<char>> strongly_connected_components(const Graph& graph) {
    CharDegreeMap indices;
    CharDegreeMap lowlinks;
    CharSet on_stack;
    CharStack stack;
    std::qvector<std::qvector<char>> components;
    std::size_t index = 0;

    std::qvector<char> nodes;
    nodes.reserve(graph.size());
    for (const auto& [node, _] : graph)
        nodes.push_back(node);
    std::sort(nodes.begin(), nodes.end());

    for (const auto node : nodes) {
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

inline AlienDictionaryResult alien_dictionary(const std::qvector<std::string>& words) {
    AlienDictionaryResult result{};

    const auto analysis = build_dependency_graph(words);
    result.has_prefix_conflict = analysis.prefix_conflict;
    result.strongly_connected = strongly_connected_components(analysis.adjacency);

    auto indegree = analysis.indegree;
    CharQueue queue;
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
            std::qvector<char> neighbors(it->second.begin(), it->second.end());
            std::sort(neighbors.begin(), neighbors.end());
            for (const auto neighbor : neighbors) {
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

inline std::string describe_components(const std::qvector<std::qvector<char>>& components) {
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

