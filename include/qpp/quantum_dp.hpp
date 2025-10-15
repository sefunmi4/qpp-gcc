#pragma once

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "qpp/qstruct.hpp"

namespace qpp::qdp {
namespace detail {

inline std::size_t qubits_for_length(std::size_t length) {
    if (length == 0)
        return 0;
    std::size_t qubits = 0;
    std::size_t capacity = 1;
    while (capacity < length) {
        capacity <<= 1U;
        ++qubits;
    }
    return qubits;
}

template <typename T>
inline double amplitude_value(std::size_t index, const T& value) {
    if constexpr (std::is_arithmetic_v<T>) {
        return static_cast<double>(value);
    } else {
        (void)value;
        return static_cast<double>(index + 1);
    }
}

} // namespace detail

/// Scalar value with a lightweight quantum representation.
template <typename T> class quantum_scalar {
public:
    quantum_scalar() : value_{} { sync_state(); }
    explicit quantum_scalar(const T& value) : value_{value} { sync_state(); }

    const T& value() const { return value_; }
    void set_value(const T& value) {
        value_ = value;
        sync_state();
    }

    const qpp::qstruct& state() const { return state_; }
    qpp::qstruct& state() { return state_; }

private:
    void sync_state() {
        state_.amplitude.clear();
        state_.amplitude.resize(1, {0.0, 0.0});
        if constexpr (std::is_same_v<T, bool>) {
            state_.amplitude[0] = {value_ ? 1.0 : 0.0, 0.0};
        } else {
            state_.amplitude[0] = {static_cast<double>(value_), 0.0};
        }
    }

    T value_;
    qpp::qstruct state_;
};

/// Quantum-aware container that mirrors std::vector behaviour.
template <typename T> class quantum_sequence {
public:
    using value_type = T;
    using container_type = std::vector<T>;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;
    using reference = typename container_type::reference;
    using const_reference = typename container_type::const_reference;

    quantum_sequence() = default;
    explicit quantum_sequence(container_type data) : data_{std::move(data)} {
        refresh_state();
    }
    quantum_sequence(std::initializer_list<T> init) : data_{init} { refresh_state(); }
    quantum_sequence(std::size_t count, const T& value = T{})
        : data_(count, value) {
        refresh_state();
    }
    template <typename It>
    quantum_sequence(It first, It last) : data_(first, last) {
        refresh_state();
    }

    std::size_t size() const { return data_.size(); }
    bool empty() const { return data_.empty(); }

    const_reference operator[](std::size_t index) const { return data_[index]; }
    reference operator[](std::size_t index) { return data_[index]; }

    iterator begin() { return data_.begin(); }
    iterator end() { return data_.end(); }
    const_iterator begin() const { return data_.begin(); }
    const_iterator end() const { return data_.end(); }
    const_iterator cbegin() const { return data_.cbegin(); }
    const_iterator cend() const { return data_.cend(); }

    const container_type& classical() const { return data_; }
    container_type& classical() { return data_; }

    void push_back(const T& value) {
        data_.push_back(value);
        refresh_state();
    }

    void set(std::size_t index, const T& value) {
        data_[index] = value;
        refresh_state();
    }

    void refresh_state() {
        const std::size_t length = data_.size();
        const std::size_t qubits = detail::qubits_for_length(length);
        const std::size_t capacity = length == 0 ? 0 : (std::size_t{1} << qubits);
        state_.amplitude.clear();
        state_.amplitude.resize(std::max<std::size_t>(capacity, 1), {0.0, 0.0});
        if (length == 0) {
            state_.amplitude[0] = {1.0, 0.0};
            return;
        }
        double norm_accumulator = 0.0;
        for (std::size_t i = 0; i < length; ++i) {
            const double base = detail::amplitude_value(i, data_[i]);
            norm_accumulator += base * base;
        }
        const double normaliser = norm_accumulator > 0.0 ? std::sqrt(norm_accumulator) : 1.0;
        for (std::size_t i = 0; i < length; ++i) {
            const double base = detail::amplitude_value(i, data_[i]);
            state_.amplitude[i] = {base / normaliser, 0.0};
        }
    }

    const qpp::qstruct& state() const { return state_; }
    qpp::qstruct& state() { return state_; }

private:
    container_type data_{};
    qpp::qstruct state_{};
};

/// Quantum-aware string helper built on top of quantum_sequence<char>.
class quantum_string : public quantum_sequence<char> {
public:
    using base = quantum_sequence<char>;
    quantum_string() = default;
    explicit quantum_string(std::string value)
        : base(value.begin(), value.end()) {}
    explicit quantum_string(std::string_view view)
        : base(view.begin(), view.end()) {}

    std::string to_std_string() const {
        return std::string(classical().begin(), classical().end());
    }

    std::string_view view() const {
        return std::string_view(classical().data(), classical().size());
    }
};

/// Quantum-aware range for index pairs.
struct quantum_index_range {
    quantum_scalar<int> left;
    quantum_scalar<int> right;

    quantum_index_range() = default;
    quantum_index_range(int l, int r) : left(l), right(r) {}

    int length() const { return right.value() - left.value(); }
};

} // namespace qpp::qdp
