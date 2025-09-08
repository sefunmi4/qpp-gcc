#ifndef QPP_BACKEND_BACKEND_HPP
#define QPP_BACKEND_BACKEND_HPP

#include <map>
#include <string>

namespace qpp {

/// Result of executing a circuit on a backend.
struct RunResult {
    bool success = true;
    std::map<std::string, double> probabilities;
    std::map<std::string, unsigned> counts;
};

/// Abstract base class for all qpp backends.
class Backend {
public:
    virtual ~Backend() = default;

    /// Returns true if the backend is available for use.
    virtual bool available() const = 0;

    /// Load a circuit description into the backend.
    virtual void loadCircuit(const std::string& circuit) = 0;

    /// Execute the loaded circuit and return the results.
    virtual RunResult run() = 0;
};

} // namespace qpp

#endif // QPP_BACKEND_BACKEND_HPP
