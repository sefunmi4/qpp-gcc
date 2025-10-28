#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>

#include "qpp/qstruct.hpp"

namespace qpp::detail::quantum_container {

inline constexpr std::size_t qubits_for_length(std::size_t length) noexcept {
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

template <typename T, typename = void> struct has_quantum_state : std::false_type {};

template <typename T>
struct has_quantum_state<T, std::void_t<decltype(std::declval<const T&>().quantum_state())>>
    : std::true_type {};

template <typename T> inline constexpr bool has_quantum_state_v = has_quantum_state<T>::value;

template <typename T> inline double amplitude_value(std::size_t index, const T& value) {
    if constexpr (std::is_arithmetic_v<T>) {
        return static_cast<double>(value);
    } else if constexpr (has_quantum_state_v<T>) {
        const auto& state = value.quantum_state();
        if (state.amplitude.empty())
            return 0.0;
        const std::size_t slot = std::min<std::size_t>(index, state.amplitude.size() - 1U);
        return std::abs(state.amplitude[slot]);
    } else {
        (void)value;
        return static_cast<double>(index + 1U);
    }
}

class propagation_state {
public:
    constexpr propagation_state() = default;

    constexpr void mark_dirty() const noexcept { amplitude_dirty_ = true; }

    template <typename Iterator>
    void ensure_amplitude(Iterator first, Iterator last) const {
        if (!amplitude_dirty_)
            return;
        update_amplitude(first, last);
    }

    constexpr void note_measurement() const noexcept { ++measurement_epoch_; }

    [[nodiscard]] constexpr std::size_t entanglement_version() const noexcept {
        return entanglement_epoch_;
    }

    [[nodiscard]] constexpr std::size_t measurement_version() const noexcept {
        return measurement_epoch_;
    }

    [[nodiscard]] const qpp::qstruct& amplitude() const noexcept { return amplitude_; }

    void swap(propagation_state& other) noexcept {
        using std::swap;
        swap(amplitude_, other.amplitude_);
        swap(amplitude_dirty_, other.amplitude_dirty_);
        swap(entanglement_epoch_, other.entanglement_epoch_);
        swap(measurement_epoch_, other.measurement_epoch_);
    }

private:
    template <typename Iterator> void update_amplitude(Iterator first, Iterator last) const {
        const std::size_t length = static_cast<std::size_t>(std::distance(first, last));
        const std::size_t qubits = qubits_for_length(length);
        const std::size_t capacity = length == 0 ? 0 : (std::size_t{1} << qubits);
        amplitude_.amplitude.assign(std::max<std::size_t>(capacity, 1U), {0.0, 0.0});
        if (length == 0) {
            amplitude_.amplitude[0] = {1.0, 0.0};
            amplitude_dirty_ = false;
            ++entanglement_epoch_;
            return;
        }

        double norm_accumulator = 0.0;
        std::size_t index = 0;
        for (Iterator it = first; it != last; ++it, ++index) {
            const double base = amplitude_value(index, *it);
            norm_accumulator += base * base;
        }
        const double normaliser = norm_accumulator > 0.0 ? std::sqrt(norm_accumulator) : 1.0;
        index = 0;
        for (Iterator it = first; it != last; ++it, ++index) {
            const double base = amplitude_value(index, *it);
            amplitude_.amplitude[index] = {base / normaliser, 0.0};
        }
        for (std::size_t i = index; i < amplitude_.amplitude.size(); ++i)
            amplitude_.amplitude[i] = {0.0, 0.0};

        amplitude_dirty_ = false;
        ++entanglement_epoch_;
    }

    mutable qpp::qstruct amplitude_{};
    mutable bool amplitude_dirty_{true};
    mutable std::size_t entanglement_epoch_{0};
    mutable std::size_t measurement_epoch_{0};
};

} // namespace qpp::detail::quantum_container

