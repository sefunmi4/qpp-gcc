#ifndef QPP_QREGISTER_HPP
#define QPP_QREGISTER_HPP

#include "qstruct.hpp"
#include <vector>
#include <cstddef>

namespace qpp {

/// Quantum register backed by qclass
class qregister {
    qclass reg;
public:
    explicit qregister(std::size_t qubits = 0) : reg(qubits) {}

    /// Access underlying quantum class
    qclass& data() { return reg; }
    const qclass& data() const { return reg; }

    /// Export state amplitudes
    std::vector<std::complex<double>> export_state() const {
        return reg.data().amplitude;
    }

    /// Import state amplitudes (size must match)
    void import_state(const std::vector<std::complex<double>>& amp) {
        reg.data().amplitude = amp;
    }
};

/// Classical register storing bits
class cregister {
    std::vector<int> bits;
public:
    explicit cregister(std::size_t size = 0) : bits(size, 0) {}

    int& operator[](std::size_t i) { return bits[i]; }
    const int& operator[](std::size_t i) const { return bits[i]; }

    std::size_t size() const { return bits.size(); }
};

} // namespace qpp

#endif // QPP_QREGISTER_HPP
