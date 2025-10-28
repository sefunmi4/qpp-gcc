#pragma once

#include <initializer_list>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "qpp/entangled_set"
#include "qpp/qint"
#include "qpp/qvector"

namespace qpp::examples::linked_list {

struct ListNode {
    qint value{};
    std::shared_ptr<ListNode> next{};
};

using NodePtr = std::shared_ptr<ListNode>;

inline qint make_quantum_value(int classical) {
    return qint{classical, classical, classical, classical};
}

inline int collapse_value(const qint& value) {
    const auto stats = value.measure_axis(0U);
    if (stats.collapsed_value)
        return *stats.collapsed_value;
    return value.x_step()();
}

inline qint collapse_register(const qint& value) {
    const auto register_stats = value.measure_register();
    const auto fetch = [&](std::size_t axis, const qint::MeasurementHandle& handle) {
        if (register_stats[axis].collapsed_value)
            return *register_stats[axis].collapsed_value;
        return handle();
    };

    const int x = fetch(0U, value.x_step());
    const int y = fetch(1U, value.y_step());
    const int z = fetch(2U, value.z_step());
    const int t = fetch(3U, value.t_step());

    return qint{x, y, z, t};
}

inline NodePtr make_node(qint value, NodePtr next = nullptr) {
    return std::make_shared<ListNode>(ListNode{std::move(value), std::move(next)});
}

inline NodePtr make_node(int value, NodePtr next = nullptr) {
    return make_node(make_quantum_value(value), std::move(next));
}

inline NodePtr make_list(qpp::qvector<qint> values) {
    if (values.empty())
        return nullptr;

    NodePtr head = make_node(std::move(values.front()));
    auto current = head;

    for (std::size_t index = 1; index < values.size(); ++index) {
        current->next = make_node(std::move(values[index]));
        current = current->next;
    }

    return head;
}

inline NodePtr make_list(const std::vector<int>& values) {
    qpp::qvector<qint> quantum_values;
    quantum_values.reserve(values.size());
    for (int value : values)
        quantum_values.emplace_back(make_quantum_value(value));
    return make_list(std::move(quantum_values));
}

inline NodePtr make_list(std::initializer_list<int> values) {
    return make_list(std::vector<int>(values));
}

inline qpp::qvector<qint> to_vector(const NodePtr& head,
                                    std::size_t max_nodes = 64,
                                    bool* truncated = nullptr) {
    qpp::qvector<qint> values;
    values.reserve(max_nodes);

    std::entangled_set<const ListNode*> visited_nodes;
    auto current = head;
    std::size_t visited = 0;

    while (current && visited < max_nodes && visited_nodes.insert(current.get()).second) {
        values.emplace_back(collapse_register(current->value));
        current = current->next;
        ++visited;
    }

    if (truncated)
        *truncated = static_cast<bool>(current);

    return values;
}

inline std::vector<int> to_classical_vector(const NodePtr& head,
                                            std::size_t max_nodes = 64,
                                            bool* truncated = nullptr) {
    const auto quantum_values = to_vector(head, max_nodes, truncated);
    std::vector<int> classical;
    classical.reserve(quantum_values.size());
    for (const auto& value : quantum_values)
        classical.push_back(collapse_value(value));
    return classical;
}

inline std::string describe_list(const NodePtr& head,
                                 std::size_t max_nodes = 16) {
    bool truncated = false;
    const auto values = to_classical_vector(head, max_nodes, &truncated);

    std::ostringstream stream;
    stream << "[";

    for (std::size_t index = 0; index < values.size(); ++index) {
        if (index > 0)
            stream << ", ";
        stream << values[index];
    }

    if (truncated)
        stream << ", ...";

    stream << "]";
    return stream.str();
}

inline NodePtr reverse_list(NodePtr head) {
    NodePtr previous = nullptr;

    while (head) {
        auto next = head->next;
        head->next = previous;
        previous = head;
        head = std::move(next);
    }

    return previous;
}

inline NodePtr merge_two_sorted_lists(NodePtr list1, NodePtr list2) {
    if (!list1)
        return list2;
    if (!list2)
        return list1;

    auto dummy = make_node(make_quantum_value(0));
    auto tail = dummy;

    while (list1 && list2) {
        if (collapse_value(list1->value) <= collapse_value(list2->value)) {
            tail->next = list1;
            tail = tail->next;
            list1 = list1->next;
        } else {
            tail->next = list2;
            tail = tail->next;
            list2 = list2->next;
        }
    }

    tail->next = list1 ? list1 : list2;
    return dummy->next;
}

inline bool has_cycle(const NodePtr& head) {
    auto slow = head;
    auto fast = head;

    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;

        if (slow && slow == fast)
            return true;
    }

    return false;
}

inline NodePtr reorder_list(NodePtr head) {
    if (!head || !head->next)
        return head;

    auto slow = head;
    auto fast = head;

    while (fast->next && fast->next->next) {
        slow = slow->next;
        fast = fast->next->next;
    }

    auto second_half = slow->next;
    slow->next.reset();
    second_half = reverse_list(second_half);

    auto first_half = head;
    while (first_half && second_half) {
        auto first_next = first_half->next;
        auto second_next = second_half->next;

        first_half->next = second_half;
        second_half->next = first_next;

        first_half = first_next;
        second_half = second_next;
    }

    return head;
}

inline NodePtr remove_nth_from_end(NodePtr head, int n) {
    auto dummy = make_node(make_quantum_value(0), head);
    auto lead = dummy;

    for (int step = 0; step < n && lead; ++step)
        lead = lead->next;

    auto lag = dummy;
    while (lead && lead->next) {
        lead = lead->next;
        lag = lag->next;
    }

    if (lag && lag->next)
        lag->next = lag->next->next;

    return dummy->next;
}

inline NodePtr merge_k_sorted_lists(qpp::qvector<NodePtr> lists) {
    struct NodePtrCompare {
        bool operator()(const NodePtr& lhs, const NodePtr& rhs) const {
            return collapse_value(lhs->value) > collapse_value(rhs->value);
        }
    };

    std::priority_queue<NodePtr, qpp::qvector<NodePtr>, NodePtrCompare> heap;
    for (auto& list : lists)
        if (list)
            heap.push(list);

    auto dummy = make_node(make_quantum_value(0));
    auto tail = dummy;

    while (!heap.empty()) {
        auto node = heap.top();
        heap.pop();

        if (node->next)
            heap.push(node->next);

        tail->next = node;
        tail = tail->next;
    }

    return dummy->next;
}

} // namespace qpp::examples::linked_list

