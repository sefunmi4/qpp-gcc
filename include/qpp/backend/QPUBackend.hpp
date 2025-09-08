#ifndef QPP_BACKEND_QPUBACKEND_HPP
#define QPP_BACKEND_QPUBACKEND_HPP

#include "Backend.hpp"

namespace qpp {

/// Stub implementation of a quantum processing unit backend.
class QPUBackend : public Backend {
public:
    bool available() const override { return false; }
    void loadCircuit(const std::string& /*circuit*/) override {}
    RunResult run() override { return {}; }
};

} // namespace qpp

#endif // QPP_BACKEND_QPUBACKEND_HPP
