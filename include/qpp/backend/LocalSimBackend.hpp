#ifndef QPP_BACKEND_LOCALSIMBACKEND_HPP
#define QPP_BACKEND_LOCALSIMBACKEND_HPP

#include <random>
#include <string>
#include <vector>

#include "Backend.hpp"
#include "../qstruct.hpp"

namespace qpp {

/// Backend representing a local simulator.
class LocalSimBackend : public Backend {
public:
    using StateVector = qclass;

    LocalSimBackend(int max_qubits = 0, bool use_fftw = false, unsigned seed = 0);

    bool available() const override { return true; }
    void loadCircuit(const std::string& circuit) override;
    RunResult run() override;

    const StateVector& state() const { return psi_; }

private:
    struct Operation {
        std::string gate;
        std::vector<int> args;
    };

    int maxQubits_ = 0;
    bool useFFTW_ = false;
    StateVector psi_{};
    std::mt19937 rng_{};
    unsigned shots_ = 0;
    std::vector<Operation> ops_;
    std::vector<int> measuredQubits_;
};

} // namespace qpp

#endif // QPP_BACKEND_LOCALSIMBACKEND_HPP
