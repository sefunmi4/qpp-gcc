#pragma once

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstddef>
#include <vector>

#include "qpp/pbool.h"
#include "qpp/qint"
#include "qpp/qstruct.hpp"
#include "qpp/qvector"

namespace qpp::examples::intervals {

struct quantum_interval {
    qpp::qint start{};
    qpp::qint end{};
};

namespace detail {

struct classical_interval {
    int start{};
    int end{};
};

inline int collapse_endpoint(const qpp::qint& endpoint) {
    const auto stats = endpoint.measure_axis(0U);
    if (stats.collapsed_value)
        return *stats.collapsed_value;
    return endpoint.x_step()();
}

inline classical_interval collapse_interval(const quantum_interval& interval) {
    return classical_interval{collapse_endpoint(interval.start),
                              collapse_endpoint(interval.end)};
}

inline std::vector<classical_interval>
collapse_intervals(const qpp::qvector<quantum_interval>& intervals) {
    std::vector<classical_interval> collapsed;
    collapsed.reserve(intervals.size());
    for (const auto& interval : intervals)
        collapsed.push_back(collapse_interval(interval));
    return collapsed;
}

inline qpp::qint make_endpoint(int value) {
    return qpp::qint{value, value, value, value};
}

inline quantum_interval expand_interval(const classical_interval& interval) {
    return quantum_interval{make_endpoint(interval.start),
                            make_endpoint(interval.end)};
}

inline qpp::qvector<quantum_interval>
expand_intervals(const std::vector<classical_interval>& intervals) {
    qpp::qvector<quantum_interval> expanded;
    expanded.reserve(intervals.size());
    for (const auto& interval : intervals)
        expanded.push_back(expand_interval(interval));
    return expanded;
}

} // namespace detail

/// Insert a new interval into a sorted list of disjoint intervals.
inline qpp::qvector<quantum_interval>
insert_interval(const qpp::qvector<quantum_interval>& intervals,
                const quantum_interval& new_interval) {
    const auto collapsed = detail::collapse_intervals(intervals);
    detail::classical_interval merged_new = detail::collapse_interval(new_interval);

    std::vector<detail::classical_interval> result;
    result.reserve(collapsed.size() + 1);

    std::size_t index = 0;
    while (index < collapsed.size() &&
           collapsed[index].end < merged_new.start) {
        result.push_back(collapsed[index]);
        ++index;
    }

    while (index < collapsed.size() &&
           collapsed[index].start <= merged_new.end) {
        merged_new.start = std::min(merged_new.start, collapsed[index].start);
        merged_new.end = std::max(merged_new.end, collapsed[index].end);
        ++index;
    }

    result.push_back(merged_new);

    while (index < collapsed.size()) {
        result.push_back(collapsed[index]);
        ++index;
    }

    return detail::expand_intervals(result);
}

/// Merge all overlapping intervals.
inline qpp::qvector<quantum_interval>
merge_intervals(const qpp::qvector<quantum_interval>& intervals) {
    auto collapsed = detail::collapse_intervals(intervals);
    if (collapsed.empty())
        return {};

    std::sort(collapsed.begin(), collapsed.end(),
              [](const detail::classical_interval& lhs,
                 const detail::classical_interval& rhs) {
                  if (lhs.start != rhs.start)
                      return lhs.start < rhs.start;
                  return lhs.end < rhs.end;
              });

    std::vector<detail::classical_interval> merged;
    merged.reserve(collapsed.size());
    merged.push_back(collapsed.front());

    for (std::size_t i = 1; i < collapsed.size(); ++i) {
        detail::classical_interval& back = merged.back();
        if (collapsed[i].start <= back.end) {
            back.end = std::max(back.end, collapsed[i].end);
        } else {
            merged.push_back(collapsed[i]);
        }
    }

    return detail::expand_intervals(merged);
}

/// Minimum number of intervals to remove so that the remainder do not overlap.
inline int
erase_overlap_intervals(const qpp::qvector<quantum_interval>& intervals) {
    if (intervals.size() < 2)
        return 0;

    auto sorted = detail::collapse_intervals(intervals);
    std::sort(sorted.begin(), sorted.end(),
              [](const detail::classical_interval& lhs,
                 const detail::classical_interval& rhs) {
                  if (lhs.end != rhs.end)
                      return lhs.end < rhs.end;
                  return lhs.start < rhs.start;
              });

    int removals = 0;
    detail::classical_interval current = sorted.front();

    for (std::size_t i = 1; i < sorted.size(); ++i) {
        if (sorted[i].start < current.end) {
            ++removals;
        } else {
            current = sorted[i];
        }
    }

    return removals;
}

/// Determine whether all meetings can be attended without overlaps.
inline bool
can_attend_all_meetings(const qpp::qvector<quantum_interval>& intervals) {
    if (intervals.size() < 2)
        return true;

    auto collapsed = detail::collapse_intervals(intervals);
    std::sort(collapsed.begin(), collapsed.end(),
              [](const detail::classical_interval& lhs,
                 const detail::classical_interval& rhs) {
                  if (lhs.start != rhs.start)
                      return lhs.start < rhs.start;
                  return lhs.end < rhs.end;
              });

    for (std::size_t i = 1; i < collapsed.size(); ++i) {
        if (collapsed[i].start < collapsed[i - 1].end)
            return false;
    }

    return true;
}

/// Minimum number of meeting rooms required to host all intervals.
inline int min_meeting_rooms(const qpp::qvector<quantum_interval>& intervals) {
    if (intervals.empty())
        return 0;

    std::vector<int> starts;
    std::vector<int> ends;
    starts.reserve(intervals.size());
    ends.reserve(intervals.size());

    const auto collapsed = detail::collapse_intervals(intervals);
    for (const auto& item : collapsed) {
        starts.push_back(item.start);
        ends.push_back(item.end);
    }

    std::sort(starts.begin(), starts.end());
    std::sort(ends.begin(), ends.end());

    std::size_t start_index = 0;
    std::size_t end_index = 0;
    int rooms = 0;
    int max_rooms = 0;

    while (start_index < starts.size()) {
        if (starts[start_index] < ends[end_index]) {
            ++rooms;
            ++start_index;
            max_rooms = std::max(max_rooms, rooms);
        } else {
            --rooms;
            ++end_index;
        }
    }

    return max_rooms;
}

/// Probability bias describing how likely a random pair of intervals overlaps.
inline qpp::pbool conflict_bias(const qpp::qvector<quantum_interval>& intervals) {
    if (intervals.size() < 2)
        return qpp::pbool{0.0};

    std::size_t overlaps = 0;
    const std::size_t total_pairs = intervals.size() * (intervals.size() - 1) / 2;

    const auto collapsed = detail::collapse_intervals(intervals);
    for (std::size_t i = 0; i < collapsed.size(); ++i) {
        for (std::size_t j = i + 1; j < collapsed.size(); ++j) {
            const bool overlap =
                !(collapsed[i].end <= collapsed[j].start ||
                  collapsed[j].end <= collapsed[i].start);
            if (overlap)
                ++overlaps;
        }
    }

    const double probability = static_cast<double>(overlaps) /
                               static_cast<double>(total_pairs);
    return qpp::pbool{probability};
}

/// Build a register encoding a uniform superposition of interval indices.
inline qpp::qclass interval_index_superposition(std::size_t count) {
    if (count == 0)
        return qpp::qclass{};

    std::size_t qubits = 0;
    while ((std::size_t{1} << qubits) < count)
        ++qubits;

    qpp::qclass reg(qubits);
    for (std::size_t q = 0; q < qubits; ++q)
        reg.apply_h(q);

    auto& amplitude = reg.data().amplitude;
    for (std::size_t basis = count; basis < amplitude.size(); ++basis)
        amplitude[basis] = {0.0, 0.0};

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

} // namespace qpp::examples::intervals

