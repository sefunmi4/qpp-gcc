#pragma once

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <vector>

#include "qpp/quantum/quantum_heap.hpp"

namespace qpp::examples::heap {

/// A streaming median data structure built on top of two heaps.
class MedianFinder {
  public:
    using QuantumHeap = qpp::quantum::QuantumHeap;

    MedianFinder()
        : MedianFinder(qpp::quantum::BackendKind::CPU, qpp::quantum::BackendConfiguration{}, 0) {}

    MedianFinder(qpp::quantum::BackendKind backend,
                 qpp::quantum::BackendConfiguration config = {},
                 std::size_t shots = 0)
        : lower_half_{QuantumHeap::Order::Max, backend, config, shots},
          upper_half_{QuantumHeap::Order::Min, backend, config, shots} {}

    template <typename InputIt>
    MedianFinder(InputIt first, InputIt last,
                 qpp::quantum::BackendKind backend = qpp::quantum::BackendKind::CPU,
                 qpp::quantum::BackendConfiguration config = {},
                 std::size_t shots = 0)
        : MedianFinder(backend, config, shots) {
        for (; first != last; ++first)
            addNum(*first);
    }

    MedianFinder(std::initializer_list<int> values,
                 qpp::quantum::BackendKind backend = qpp::quantum::BackendKind::CPU,
                 qpp::quantum::BackendConfiguration config = {},
                 std::size_t shots = 0)
        : MedianFinder(values.begin(), values.end(), backend, config, shots) {}

    /// Configure both quantum heaps with a shared backend and sampling policy.
    void configure(qpp::quantum::BackendKind backend,
                   qpp::quantum::BackendConfiguration config = {},
                   std::size_t shots = 0) {
        lower_half_.configure_backend(backend, config, shots);
        upper_half_.configure_backend(backend, config, shots);
    }

    /// Add a number from the stream to the data structure.
    void addNum(int num) {
        if (lower_half_.empty() || num <= lower_half_.top())
            lower_half_.push(num);
        else
            upper_half_.push(num);

        rebalance();
    }

    /// Return the median of all numbers seen so far.
    [[nodiscard]] double findMedian() const {
        if (lower_half_.empty() && upper_half_.empty())
            throw std::logic_error("MedianFinder::findMedian: no numbers have been added");

        if (lower_half_.size() == upper_half_.size())
            return (static_cast<double>(lower_half_.top()) +
                    static_cast<double>(upper_half_.top())) /
                   2.0;

        return (lower_half_.size() > upper_half_.size())
                   ? static_cast<double>(lower_half_.top())
                   : static_cast<double>(upper_half_.top());
    }

    /// Return the number of elements contained in the structure.
    [[nodiscard]] std::size_t size() const noexcept {
        return lower_half_.size() + upper_half_.size();
    }

  private:
    void rebalance() {
        if (lower_half_.size() > upper_half_.size() + 1) {
            lower_half_.transfer_top_to(upper_half_);
        } else if (upper_half_.size() > lower_half_.size() + 1) {
            upper_half_.transfer_top_to(lower_half_);
        }
    }

    QuantumHeap lower_half_;
    QuantumHeap upper_half_;
};

/// Compute the running median for each prefix of the input sequence.
inline std::vector<double> compute_running_medians(
    const std::vector<int>& values) {
    MedianFinder finder;
    std::vector<double> medians;
    medians.reserve(values.size());

    for (int value : values) {
        finder.addNum(value);
        medians.push_back(finder.findMedian());
    }

    return medians;
}

} // namespace qpp::examples::heap

