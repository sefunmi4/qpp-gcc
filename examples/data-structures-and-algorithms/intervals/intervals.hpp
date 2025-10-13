#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <vector>

#include "qpp/pbool.h"
#include "qpp/qstruct.hpp"

namespace qpp::examples::intervals {

using interval = std::array<int, 2>;

/// Insert a new interval into a sorted list of disjoint intervals.
inline std::vector<interval> insert_interval(std::vector<interval> intervals,
                                             interval new_interval) {
    std::vector<interval> result;
    result.reserve(intervals.size() + 1);

    std::size_t index = 0;
    while (index < intervals.size() &&
           intervals[index][1] < new_interval[0]) {
        result.push_back(intervals[index]);
        ++index;
    }

    while (index < intervals.size() &&
           intervals[index][0] <= new_interval[1]) {
        new_interval[0] = std::min(new_interval[0], intervals[index][0]);
        new_interval[1] = std::max(new_interval[1], intervals[index][1]);
        ++index;
    }

    result.push_back(new_interval);

    while (index < intervals.size()) {
        result.push_back(intervals[index]);
        ++index;
    }

    return result;
}

/// Merge all overlapping intervals.
inline std::vector<interval> merge_intervals(std::vector<interval> intervals) {
    if (intervals.empty())
        return {};

    std::sort(intervals.begin(), intervals.end(),
              [](const interval& lhs, const interval& rhs) {
                  if (lhs[0] != rhs[0])
                      return lhs[0] < rhs[0];
                  return lhs[1] < rhs[1];
              });

    std::vector<interval> merged;
    merged.reserve(intervals.size());
    merged.push_back(intervals.front());

    for (std::size_t i = 1; i < intervals.size(); ++i) {
        interval& back = merged.back();
        if (intervals[i][0] <= back[1]) {
            back[1] = std::max(back[1], intervals[i][1]);
        } else {
            merged.push_back(intervals[i]);
        }
    }

    return merged;
}

/// Minimum number of intervals to remove so that the remainder do not overlap.
inline int erase_overlap_intervals(const std::vector<interval>& intervals) {
    if (intervals.size() < 2)
        return 0;

    std::vector<interval> sorted = intervals;
    std::sort(sorted.begin(), sorted.end(),
              [](const interval& lhs, const interval& rhs) {
                  if (lhs[1] != rhs[1])
                      return lhs[1] < rhs[1];
                  return lhs[0] < rhs[0];
              });

    int removals = 0;
    interval current = sorted.front();

    for (std::size_t i = 1; i < sorted.size(); ++i) {
        if (sorted[i][0] < current[1]) {
            ++removals;
        } else {
            current = sorted[i];
        }
    }

    return removals;
}

/// Determine whether all meetings can be attended without overlaps.
inline bool can_attend_all_meetings(std::vector<interval> intervals) {
    if (intervals.size() < 2)
        return true;

    std::sort(intervals.begin(), intervals.end(),
              [](const interval& lhs, const interval& rhs) {
                  if (lhs[0] != rhs[0])
                      return lhs[0] < rhs[0];
                  return lhs[1] < rhs[1];
              });

    for (std::size_t i = 1; i < intervals.size(); ++i) {
        if (intervals[i][0] < intervals[i - 1][1])
            return false;
    }

    return true;
}

/// Minimum number of meeting rooms required to host all intervals.
inline int min_meeting_rooms(const std::vector<interval>& intervals) {
    if (intervals.empty())
        return 0;

    std::vector<int> starts;
    std::vector<int> ends;
    starts.reserve(intervals.size());
    ends.reserve(intervals.size());

    for (const auto& item : intervals) {
        starts.push_back(item[0]);
        ends.push_back(item[1]);
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
inline qpp::pbool conflict_bias(const std::vector<interval>& intervals) {
    if (intervals.size() < 2)
        return qpp::pbool{0.0};

    std::size_t overlaps = 0;
    const std::size_t total_pairs = intervals.size() * (intervals.size() - 1) / 2;

    for (std::size_t i = 0; i < intervals.size(); ++i) {
        for (std::size_t j = i + 1; j < intervals.size(); ++j) {
            const bool overlap =
                !(intervals[i][1] <= intervals[j][0] ||
                  intervals[j][1] <= intervals[i][0]);
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

