#ifndef QPP_API_PROGRAM_HPP
#define QPP_API_PROGRAM_HPP

#include <memory>
#include <random>
#include <string>

#include "qpp/api/Circuit.hpp"
#include "qpp/api/Sampler.hpp"
#include "qpp/backend/Backend.hpp"
#include "qpp/backend/Factory.hpp"

namespace qpp {
namespace api {

/// Program ties a Circuit to execution backends.
class Program {
public:
    explicit Program(const Circuit& c)
        : circuit_(c), backend_(BackendFactory::create()) {}

    Program(const Circuit& c, std::unique_ptr<Backend> backend)
        : circuit_(c), backend_(std::move(backend)) {}

    // Legacy constructor accepting explicit backend pointers.
    //\note The QPU pointer is ignored and retained for backwards
    // compatibility with older examples that provided both a hardware and
    // simulator backend. If a simulator pointer is supplied it is copied into
    // an internally owned instance; otherwise the factory selection is used.
    Program(const Circuit& c, Backend*, LocalSimBackend* sim)
        : circuit_(c),
          backend_(sim ? std::make_unique<LocalSimBackend>(*sim)
                        : BackendFactory::create()) {}

    /// Execute the circuit on the selected backend.
    RunResult execute() {
        std::string text = circuit_.toString();
        RunResult result;
        if (backend_ && backend_->available()) {
            backend_->loadCircuit(text);
            result = backend_->run();
        }
        if (circuit_.shots > 0 && result.counts.empty() && !result.probabilities.empty())
            result.counts = Sampler::sample(result.probabilities, circuit_.shots, rng_);
        return result;
    }

    Circuit circuit_;

private:
    std::unique_ptr<Backend> backend_;
    std::mt19937 rng_{std::random_device{}()};
};

} // namespace api
} // namespace qpp

#endif // QPP_API_PROGRAM_HPP
