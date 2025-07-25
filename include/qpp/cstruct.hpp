#ifndef QPP_CSTRUCT_HPP
#define QPP_CSTRUCT_HPP

#include <vector>
#include <cstddef>

namespace qpp {

/// Purely classical state container
struct cstruct {
    std::vector<int> bits;

    explicit cstruct(std::size_t n = 0) : bits(n, 0) {}

    std::size_t size() const { return bits.size(); }
};

/// Wrapper class similar to qclass but for classical bits
class cclass {
    cstruct state;
public:
    explicit cclass(std::size_t n = 0) : state(n) {}

    cstruct& data() { return state; }
    const cstruct& data() const { return state; }
};

} // namespace qpp

#endif // QPP_CSTRUCT_HPP
