#ifndef QPP_BACKEND_LOCALSIMBACKEND_HPP
#define QPP_BACKEND_LOCALSIMBACKEND_HPP

#include "Backend.hpp"

namespace qpp {

/// Backend representing a local simulator.
class LocalSimBackend : public Backend {
public:
    bool available() const override;
    void loadCircuit(const std::string& circuit) override;
    RunResult run() override;
};

} // namespace qpp

#endif // QPP_BACKEND_LOCALSIMBACKEND_HPP
