#include <cassert>

#include "examples/data-structures-and-algorithms/dynamic-programming-1d/dynamic_programming_1d.hpp"

int main() {
    using namespace qpp::examples::dynamic_programming_1d;
    using qpp::qdp::quantum_scalar;
    using qpp::qdp::quantum_sequence;
    using qpp::qdp::quantum_string;

    const auto stairs = climb_stairs(quantum_scalar<int>(6));
    assert(stairs.value() == 13);

    const quantum_sequence<int> houses{1, 2, 3, 1};
    const auto robber = house_robber(houses);
    assert(robber.value() == 4);

    const quantum_string decode(std::string{"11106"});
    const auto decoding = decode_ways(decode);
    assert(decoding.value() == 2);

    const quantum_sequence<int> coins{1, 3, 4};
    const auto change = coin_change(coins, quantum_scalar<int>(6));
    assert(change.value() == 2);

    const quantum_sequence<std::string> dictionary{"leet", "code"};
    const auto break_result =
        word_break(quantum_string(std::string{"leetcode"}), dictionary);
    assert(break_result.value());

    return 0;
}
