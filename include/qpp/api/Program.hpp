#ifndef QPP_API_PROGRAM_HPP
#define QPP_API_PROGRAM_HPP

#include <random>
#include <string>

#include "qpp/api/Circuit.hpp"
#include "qpp/api/Sampler.hpp"
#include "qpp/backend/Backend.hpp"
#include "qpp/backend/LocalSimBackend.hpp"

namespace qpp {
namespace api {

/// Program ties a Circuit to execution backends.
class Program {
public:
    Program(const Circuit& c, Backend* qpu, LocalSimBackend* sim)
        : circuit_(c), qpuBackend_(qpu), simBackend_(sim) {}

    /// Execute the circuit on the available backend.
    RunResult execute() {
        std::string text = circuit_.toString();
        RunResult result;
        if (qpuBackend_ && qpuBackend_->available()) {
            qpuBackend_->loadCircuit(text);
            result = qpuBackend_->run();
        } else if (simBackend_) {
            simBackend_->loadCircuit(text);
            result = simBackend_->run();
        }
        if (circuit_.shots > 0 && result.counts.empty() && !result.probabilities.empty())
            result.counts = Sampler::sample(result.probabilities, circuit_.shots, rng_);
        return result;
    }

    Circuit circuit_;

private:
    Backend* qpuBackend_ = nullptr;
    LocalSimBackend* simBackend_ = nullptr;
    std::mt19937 rng_{std::random_device{}()};
};

} // namespace api
} // namespace qpp

#endif // QPP_API_PROGRAM_HPP
