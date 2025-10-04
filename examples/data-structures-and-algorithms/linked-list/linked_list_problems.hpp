#pragma once

#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace qpp::examples::linked_list {

struct ListNode {
    int value{};
    std::shared_ptr<ListNode> next{};
};

using NodePtr = std::shared_ptr<ListNode>;

inline NodePtr make_node(int value, NodePtr next = nullptr) {
    return std::make_shared<ListNode>(ListNode{value, std::move(next)});
}

inline NodePtr make_list(const std::vector<int>& values) {
    if (values.empty())
        return nullptr;

    NodePtr head = make_node(values.front());
    auto current = head;

    for (std::size_t index = 1; index < values.size(); ++index) {
        current->next = make_node(values[index]);
        current = current->next;
    }

    return head;
}

inline std::vector<int> to_vector(const NodePtr& head, std::size_t max_nodes = 64,
                                  bool* truncated = nullptr) {
    std::vector<int> values;
    values.reserve(max_nodes);

    auto current = head;
    std::size_t visited = 0;
    while (current && visited < max_nodes) {
        values.push_back(current->value);
        current = current->next;
        ++visited;
    }

    if (truncated)
        *truncated = static_cast<bool>(current);

    return values;
}

inline std::string describe_list(const NodePtr& head,
                                 std::size_t max_nodes = 16) {
    bool truncated = false;
    const auto values = to_vector(head, max_nodes, &truncated);

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

    auto dummy = make_node(0);
    auto tail = dummy;

    while (list1 && list2) {
        if (list1->value <= list2->value) {
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
    auto dummy = make_node(0, head);
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

inline NodePtr merge_k_sorted_lists(std::vector<NodePtr> lists) {
    struct NodePtrCompare {
        bool operator()(const NodePtr& lhs, const NodePtr& rhs) const {
            return lhs->value > rhs->value;
        }
    };

    std::priority_queue<NodePtr, std::vector<NodePtr>, NodePtrCompare> heap;
    for (auto& list : lists)
        if (list)
            heap.push(list);

    auto dummy = make_node(0);
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

