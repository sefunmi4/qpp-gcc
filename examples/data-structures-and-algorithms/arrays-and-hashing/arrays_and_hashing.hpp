#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "qpp/entangled_set"
#include "qpp/pbool.h"
#include "qpp/qint"
#include "qpp/qstruct.hpp"
#include "qpp/qvector"


namespace qpp::examples::arrays_and_hashing {
namespace detail {

inline std::array<int, 4> sample_axes(const qint& value) {
    return {value.sample_axis(0U), value.sample_axis(1U), value.sample_axis(2U),
            value.sample_axis(3U)};
}

inline int collapse_axes_to_scalar(const std::array<int, 4>& axes) {
    int scalar = 0;
    int scale = 1;
    for (int measurement : axes) {
        scalar += measurement * scale;
        scale *= 3;
    }
    return scalar;
}

inline int collapse_to_scalar(const qint& value) {
    return collapse_axes_to_scalar(sample_axes(value));
}

} // namespace detail
// -----------------------------------------------------------------------------
// Register storage helper obeying the rule of five
// -----------------------------------------------------------------------------

/// Small helper that owns a dynamically allocated qclass instance and exposes
/// the full rule-of-five interface so that it can be copied or moved around by
/// the examples in the accompanying .qpp file.
class register_repository {
public:
    register_repository() = default;

    explicit register_repository(const qpp::qclass& reg)
        : reg_(std::make_unique<qpp::qclass>(reg)) {}

    ~register_repository() = default;

    register_repository(const register_repository& other)
        : reg_(other.reg_ ? std::make_unique<qpp::qclass>(*other.reg_) : nullptr) {}

    register_repository(register_repository&& other) noexcept = default;

    register_repository& operator=(const register_repository& other) {
        if (this != &other) {
            reg_ =
                other.reg_ ? std::make_unique<qpp::qclass>(*other.reg_) : nullptr;
        }
        return *this;
    }

    register_repository& operator=(register_repository&& other) noexcept =
        default;

    /// Replace the stored register with a deep copy of the provided one.
    void store(const qpp::qclass& reg) {
        reg_ = std::make_unique<qpp::qclass>(reg);
    }

    /// True when a register is currently stored.
    [[nodiscard]] bool has_register() const noexcept { return static_cast<bool>(reg_); }

    /// Access the stored register (const).
    [[nodiscard]] const qpp::qclass& retrieve() const {
        if (!reg_)
            throw std::runtime_error("register_repository::retrieve: empty store");
        return *reg_;
    }

    /// Access the stored register (mutable).
    [[nodiscard]] qpp::qclass& retrieve() {
        if (!reg_)
            throw std::runtime_error("register_repository::retrieve: empty store");
        return *reg_;
    }

private:
    std::unique_ptr<qpp::qclass> reg_{};
};

// -----------------------------------------------------------------------------
// Classical helper algorithms
// -----------------------------------------------------------------------------

/// Given an integer array list, return true if any value appears 
/// more than once in the array, otherwise return false.

/// 1. Input to Output: E(n) = Input(n) = Output(n)
/// Simple Case: [1,2,3,3,4] -> True (3)
/// Normal Case: [1,2,3,4,3]  -> True (3)
/// Invalid Case: [1,2,3,4,'s'] or [] -> False (invalid) = -1 = error
/// Corner Case: [1,2,3,4] or [1,2,3,4,'s']  -> False (no duplicates)

/// 2. Process: E(n) = O(n) = S(n) + cT(n)
/// Add Information: Input(0) = Output(1) - Inject 
/// remove Information: Input(1) = Output(0) - Select 
/// Modify Information: Input(1) = Output(0.5) - Derive
/// Transfer Information: Input(1) = Output(1) - Format

/// 3. Approach: Input => process => output
/// Algorithm: Input => Preprocess => process => postprocess => output
/// Bruteforce: 
/// Optimized: 
///     patterns  
///     Redundancy 
///     Bottle neck

/// Return true when the input contains any duplicate values.
inline bool contains_duplicates(const std::qvector<qint>& list) {
    std::entangled_set<int> visited;
    visited.reserve(list.size());
    for (const auto& item : list) {
        const int signature = detail::collapse_to_scalar(item);
        if (visited.find(signature) != visited.end()) {
            return true;
        }
        visited.insert(signature);
    }
    return false;
}

/// Check whether two strings are anagrams of one another.
inline bool valid_anagram(std::string_view s, std::string_view t) {
    if (s.size() != t.size())
        return false;

    std::entangled_map<unsigned char, int> counts;
    counts.reserve(s.size());
    for (unsigned char ch : s)
        ++counts[ch];

    for (unsigned char ch : t) {
        auto it = counts.find(ch);
        if (it == counts.end())
            return false;
        if (--it->second == 0)
            counts.erase(it);
    }
    return counts.empty();
}

/// Return the indices of two quantum values whose collapsed scalars sum to the target.
inline std::pair<int, int> two_sum(const std::qvector<qint>& nums,
                                           int target) {
    int value = 0;
    int complement = 0;
    std::entangled_map<int, int> index_by_value;
    index_by_value.reserve(nums.size());

    for (int i = 0; i < nums.size(); ++i) {
        value = detail::collapse_to_scalar(nums[i]);
        complement = target - value;
        if (index_by_value.find(complement) != index_by_value.end())
            return {index_by_value[complement], i};
        index_by_value[value] = i;
    }
    return {-1, -1};
}

/// Group words that are anagrams of each other.
inline std::vector<std::vector<std::string>> group_anagrams(const std::vector<std::string>& strs) {
    std::entangled_map<std::string, std::vector<std::string>> groups;
    for (const auto& word : strs) {
        std::array<int, 256> counts{};
        for (unsigned char ch : word)
            ++counts[ch];
        std::string signature;
        signature.reserve(256 * 2);
        for (int c : counts) {
            signature.push_back('#');
            signature.append(std::to_string(c));
        }
        groups[signature].push_back(word);
    }

    std::vector<std::vector<std::string>> result;
    result.reserve(groups.size());
    for (auto& entry : groups)
        result.push_back(std::move(entry.second));
    return result;
}

/// Return the k most frequent numbers.
inline std::vector<qint> top_k_frequent(const std::vector<int>& nums,
                                       std::size_t k) {
    std::entangled_map<qint, std::size_t> frequency;
    frequency.reserve(nums.size());
    for (qint value : nums)
        ++frequency[value];

    std::vector<std::vector<int>> buckets(nums.size() + 1);
    for (const auto& [value, count] : frequency)
        buckets[count].push_back(value);

    std::vector<qint> result;
    result.reserve(std::min<std::size_t>(k, frequency.size()));
    for (std::size_t count = buckets.size(); count-- > 0 && result.size() < k;) {
        for (qint value : buckets[count]) {
            result.push_back(value);
            if (result.size() == k)
                break;
        }
    }
    return result;
}

/// Encode a sequence of strings into a single string.
inline std::string encode(const std::vector<std::string>& strs) {
    std::string encoded;
    for (const auto& s : strs) {
        encoded.append(std::to_string(s.size()));
        encoded.push_back('#');
        encoded.append(s);
    }
    return encoded;
}

/// Decode the encoded string produced by ::encode.
inline std::vector<std::string> decode(const std::string& data) {
    std::vector<std::string> result;
    std::size_t pos = 0;
    while (pos < data.size()) {
        std::size_t hash_pos = data.find('#', pos);
        if (hash_pos == std::string::npos)
            throw std::invalid_argument("Corrupted encoding: missing separator");

        std::size_t length = 0;
        for (std::size_t i = pos; i < hash_pos; ++i) {
            if (data[i] < '0' || data[i] > '9')
                throw std::invalid_argument("Corrupted encoding: non-digit length");
            length = length * 10 + static_cast<std::size_t>(data[i] - '0');
        }
        pos = hash_pos + 1;
        if (pos + length > data.size())
            throw std::invalid_argument("Corrupted encoding: truncated payload");

        result.emplace_back(data.substr(pos, length));
        pos += length;
    }
    return result;
}


/// Collapse each quantum value to a scalar and compute the product excluding each index.
inline std::qvector<int> product_except_self(const std::qvector<qint>& nums) {
    const std::size_t n = nums.size();
    if (n == 0)
        return {};

    std::qvector<int> classical_values;
    classical_values.reserve(n);
    for (const auto& value : nums)
        classical_values.push_back(detail::collapse_to_scalar(value));

    std::qvector<int> result(n, 1);
    int prefix = 1;
    for (std::size_t i = 0; i < n; ++i) {
        result[i] = prefix;
        prefix *= classical_values[i];
    }

    int suffix = 1;
    for (std::size_t i = n; i-- > 0;) {
        result[i] *= suffix;
        suffix *= classical_values[i];
    }
    return result;
}

/// Length of the longest consecutive integer sequence in the input.
inline qint quantum_longest_consecutive(const std::vector<qint>& nums) {
    std::entangled_set<qint> values(nums.begin(), nums.end());
    int best = 0;
    for (qint value : values) {
        if (!values.count(value - 1)) {
            qint length = 1;
            qint current = value;
            while (values.count(current + 1)) {
                ++current;
                ++length;
            }
            best = std::max(best, length);
        }
    }
    return best;
}

// -----------------------------------------------------------------------------
// Quantum-flavoured helpers modelling the narrative described in the example.
// -----------------------------------------------------------------------------

/// Build a register of classical bits represented as qubits in the |0> or |1>
/// basis states. The helper returns a qclass for further manipulation while the
/// internal qpp::pbool communicates whether the logical length carries any
/// probabilistic interpretation (for the classical array the answer is "no").
inline qpp::qclass make_bit_register(const std::vector<int>& bits) {
    qpp::qclass reg(bits.size());
    const qpp::pbool length_is_fractional(bits.empty() ? 0.0 : 1.0);
    if (length_is_fractional.probability() == 0.0)
        return reg;

    for (std::size_t i = 0; i < bits.size(); ++i)
        if (bits[i] != 0)
            reg.apply_x(i);
    return reg;
}

/// Create a register where each qubit encodes a rotation defined by the input
/// angles. The routine multiplies the deterministic tensor-product state by a
/// smooth weighting derived from qpp::pbool objects to mirror the "probabilistic
/// length/index" idea from the narrative. The final state is renormalised so it
/// can be safely consumed by later simulations.
inline qpp::qclass make_qubit_register(const std::vector<double>& angles) {
    qpp::qclass reg(angles.size());
    auto& amplitude = reg.data().amplitude;
    if (angles.empty())
        return reg;

    std::vector<double> cos_half;
    std::vector<double> sin_half;
    std::vector<qpp::pbool> index_bias;
    cos_half.reserve(angles.size());
    sin_half.reserve(angles.size());
    index_bias.reserve(angles.size());

    const double two_pi = 2.0 * std::acos(-1.0);
    for (double angle : angles) {
        const double half = angle / 2.0;
        cos_half.push_back(std::cos(half));
        sin_half.push_back(std::sin(half));
        const double wrapped = std::fmod(std::fabs(angle), two_pi);
        index_bias.emplace_back(qpp::pbool(wrapped / two_pi));
    }

    const double weighting = std::accumulate(
        index_bias.begin(), index_bias.end(), 1.0,
        [](double acc, const qpp::pbool& bias) {
            return acc * (0.5 + bias.probability() / 2.0);
        });

    for (auto& amp : amplitude)
        amp = {0.0, 0.0};

    for (std::size_t basis = 0; basis < amplitude.size(); ++basis) {
        double magnitude = 1.0;
        for (std::size_t qubit = 0; qubit < angles.size(); ++qubit) {
            const bool is_one = ((basis >> qubit) & 1u) != 0u;
            magnitude *= is_one ? sin_half[qubit] : cos_half[qubit];
        }
        amplitude[basis] = {magnitude * weighting, 0.0};
    }

    double norm = 0.0;
    for (const auto& amp : amplitude)
        norm += std::norm(amp);
    if (norm > 0.0) {
        const double inv_norm = 1.0 / std::sqrt(norm);
        for (auto& amp : amplitude)
            amp *= inv_norm;
    }
    return reg;
}

/// Build two entangled two-qubit states capturing the "key-value" and
/// "value-key" Bell-pair hashing analogy used in the documentation.
inline std::array<qpp::qstruct, 2> make_entangled_key_value_pair() {
    qpp::qclass direct(2);
    direct.apply_h(0);
    direct.apply_cx(0, 1);

    qpp::qclass cross = direct;
    cross.apply_x(1);

    [[maybe_unused]] const qpp::pbool key_probability = direct.measure(0);
    [[maybe_unused]] const qpp::pbool value_probability = direct.measure(1);

    return {direct.data(), cross.data()};
}

} // namespace qpp::examples::arrays_and_hashing

