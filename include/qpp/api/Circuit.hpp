#ifndef QPP_API_CIRCUIT_HPP
#define QPP_API_CIRCUIT_HPP

#include <string>
#include <vector>
#include <sstream>

namespace qpp {
namespace api {

/// Representation of a quantum circuit consisting of a list of gates.
struct Gate {
    std::string op;              ///< Gate operation code
    std::vector<int> qubits;     ///< Target qubits
    std::vector<double> params;  ///< Optional gate parameters
};

/// Simple circuit container supporting qubit allocation and gate recording.
struct Circuit {
    int numQubits = 0;                 ///< Number of allocated qubits
    std::vector<Gate> gates;           ///< Sequence of gates
    unsigned shots = 0;                ///< Optional shots configuration

    /// Allocate additional qubits and return first allocated index.
    int allocateQubits(int n) {
        int start = numQubits;
        numQubits += n;
        return start;
    }

    /// Append a gate to the circuit.
    void addGate(const std::string& op, const std::vector<int>& qs,
                 const std::vector<double>& ps = {}) {
        gates.push_back({op, qs, ps});
    }

    /// Convert circuit to a textual representation understood by backends.
    std::string toString() const {
        std::ostringstream ss;
        if (shots > 0)
            ss << "shots " << shots << ';';
        for (const auto& g : gates) {
            ss << g.op;
            for (int q : g.qubits)
                ss << ' ' << q;
            for (double p : g.params)
                ss << ' ' << p;
            ss << ';';
        }
        return ss.str();
    }
};

} // namespace api
} // namespace qpp

#endif // QPP_API_CIRCUIT_HPP
