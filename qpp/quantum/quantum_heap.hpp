#pragma once

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "qpp/quantum/backend.hpp"
#include "qpp/quantum_worlds.hpp"

namespace qpp::quantum {

/// Quantum-aware heap that delegates ordering decisions to a backend.
class QuantumHeap {
  public:
    enum class Order { Max, Min };

    using value_type = int;

    QuantumHeap(Order order = Order::Max,
                BackendKind backend = BackendKind::CPU,
                BackendConfiguration config = {},
                std::size_t shots = 0);

    QuantumHeap(const QuantumHeap& other);
    QuantumHeap(QuantumHeap&& other) noexcept = default;
    QuantumHeap& operator=(const QuantumHeap& other);
    QuantumHeap& operator=(QuantumHeap&& other) noexcept = default;

    ~QuantumHeap();

    [[nodiscard]] bool empty() const noexcept { return data_.empty(); }
    [[nodiscard]] std::size_t size() const noexcept { return data_.size(); }
    [[nodiscard]] const value_type& top() const;

    void push(value_type value);
    value_type pop();
    void clear() noexcept;

    bool transfer_top_to(QuantumHeap& other);

    void configure_backend(BackendKind backend, BackendConfiguration config,
                           std::size_t shots = 0);

    void set_shots(std::size_t shots) noexcept { shots_ = shots; }
    [[nodiscard]] std::size_t shots() const noexcept { return shots_; }
    [[nodiscard]] BackendKind backend_kind() const noexcept { return backend_kind_; }

  private:
    void sift_up(std::size_t index);
    void sift_down(std::size_t index);
    bool should_promote(std::size_t parent, std::size_t child);
    bool prefers_rhs(value_type lhs, value_type rhs);
    void ensure_backend() const;

    Order order_;
    BackendKind backend_kind_;
    BackendConfiguration backend_config_;
    std::size_t shots_;
    std::vector<value_type> data_;
    mutable std::unique_ptr<QubitBackend> backend_;
};

} // namespace qpp::quantum

