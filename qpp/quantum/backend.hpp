#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace qpp {
namespace quantum {

enum class BackendKind;

struct BackendConfiguration {
    unsigned seed = 0;
    std::size_t max_qubits = 0;
    bool amplitude_output = false;

    struct OpenQasmHttpConfiguration {
        std::string endpoint;
        std::string api_token;
        std::string backend;
        std::unordered_map<std::string, std::string> headers;
        std::function<std::vector<double>(const std::string &, std::size_t,
                                          const OpenQasmHttpConfiguration &)
                      > submission_callback;

        bool enabled() const noexcept { return !endpoint.empty(); }
    } openqasm;

    std::unordered_map<std::string, std::string> user_data;
};

class QubitBackend {
  public:
    virtual ~QubitBackend() = default;

    virtual void reset() = 0;

    virtual std::size_t allocate_qubit() = 0;

    virtual void release_qubit(std::size_t id) = 0;

    virtual void apply_single_qubit_gate(const std::string &gate, std::size_t target,
                                         const std::vector<double> &params = {}) = 0;

    virtual void apply_two_qubit_gate(const std::string &gate, std::size_t control,
                                      std::size_t target,
                                      const std::vector<double> &params = {}) = 0;

    virtual void submit(std::size_t shots) = 0;

    virtual std::vector<double> retrieve_results() = 0;

    virtual std::vector<double>
    run_distribution_experiment(const std::vector<double> &weights, std::size_t shots);
};

std::unique_ptr<QubitBackend> make_backend(BackendKind kind, BackendConfiguration config);

} // namespace quantum
} // namespace qpp

