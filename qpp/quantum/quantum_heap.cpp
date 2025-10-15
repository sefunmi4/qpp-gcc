#include "qpp/quantum/quantum_heap.hpp"

#include <algorithm>

namespace qpp::quantum {

namespace {
[[nodiscard]] std::vector<double> build_weights(QuantumHeap::Order order, int lhs, int rhs) {
    if (order == QuantumHeap::Order::Max)
        return {static_cast<double>(lhs), static_cast<double>(rhs)};
    return {static_cast<double>(-lhs), static_cast<double>(-rhs)};
}
} // namespace

QuantumHeap::QuantumHeap(Order order, BackendKind backend, BackendConfiguration config,
                         std::size_t shots)
    : order_{order},
      backend_kind_{backend},
      backend_config_{std::move(config)},
      shots_{shots},
      data_{},
      backend_{} {
    ensure_backend();
}

QuantumHeap::QuantumHeap(const QuantumHeap& other)
    : order_{other.order_},
      backend_kind_{other.backend_kind_},
      backend_config_{other.backend_config_},
      shots_{other.shots_},
      data_{other.data_},
      backend_{} {
    ensure_backend();
}

QuantumHeap& QuantumHeap::operator=(const QuantumHeap& other) {
    if (this == &other)
        return *this;
    order_ = other.order_;
    backend_kind_ = other.backend_kind_;
    backend_config_ = other.backend_config_;
    shots_ = other.shots_;
    data_ = other.data_;
    backend_.reset();
    ensure_backend();
    return *this;
}

QuantumHeap::~QuantumHeap() = default;

const QuantumHeap::value_type& QuantumHeap::top() const {
    if (data_.empty())
        throw std::logic_error("QuantumHeap::top: heap is empty");
    return data_.front();
}

void QuantumHeap::push(value_type value) {
    data_.push_back(value);
    if (data_.size() == 1)
        return;
    sift_up(data_.size() - 1);
}

QuantumHeap::value_type QuantumHeap::pop() {
    if (data_.empty())
        throw std::logic_error("QuantumHeap::pop: heap is empty");

    value_type result = data_.front();
    data_.front() = data_.back();
    data_.pop_back();
    if (!data_.empty())
        sift_down(0);
    return result;
}

void QuantumHeap::clear() noexcept {
    data_.clear();
}

bool QuantumHeap::transfer_top_to(QuantumHeap& other) {
    if (empty())
        return false;
    other.push(pop());
    return true;
}

void QuantumHeap::configure_backend(BackendKind backend, BackendConfiguration config,
                                    std::size_t shots) {
    backend_kind_ = backend;
    backend_config_ = std::move(config);
    shots_ = shots;
    backend_.reset();
    ensure_backend();
}

void QuantumHeap::sift_up(std::size_t index) {
    while (index > 0) {
        std::size_t parent = (index - 1) / 2;
        if (!should_promote(parent, index))
            break;
        std::swap(data_[parent], data_[index]);
        index = parent;
    }
}

void QuantumHeap::sift_down(std::size_t index) {
    const std::size_t size = data_.size();
    while (true) {
        std::size_t left = index * 2 + 1;
        std::size_t right = index * 2 + 2;
        std::size_t candidate = index;

        if (left < size && should_promote(candidate, left))
            candidate = left;
        if (right < size && should_promote(candidate, right))
            candidate = right;

        if (candidate == index)
            break;

        std::swap(data_[index], data_[candidate]);
        index = candidate;
    }
}

bool QuantumHeap::should_promote(std::size_t parent, std::size_t child) {
    return prefers_rhs(data_[parent], data_[child]);
}

bool QuantumHeap::prefers_rhs(value_type lhs, value_type rhs) {
    ensure_backend();

    auto weights = build_weights(order_, lhs, rhs);
    auto result = backend_->run_distribution_experiment(weights, shots_);
    if (result.size() >= 2)
        return result.front() < result.back();

    if (order_ == Order::Max)
        return lhs < rhs;
    return lhs > rhs;
}

void QuantumHeap::ensure_backend() const {
    if (backend_)
        return;
    BackendConfiguration config_copy = backend_config_;
    backend_ = make_backend(backend_kind_, std::move(config_copy));
}

} // namespace qpp::quantum

