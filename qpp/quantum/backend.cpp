#include "qpp/quantum/backend.hpp"

#include "qpp/quantum_worlds.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <sstream>
#include <stdexcept>

namespace qpp {
namespace quantum {

std::vector<double> QubitBackend::run_distribution_experiment(
    const std::vector<double> &weights, std::size_t shots) {
    (void)weights;
    (void)shots;
    throw std::logic_error("distribution experiment not implemented for backend");
}

namespace {

struct GateOperation {
    enum class Kind { Single, Two };

    Kind kind;
    std::string gate;
    std::size_t control = 0;
    std::size_t target = 0;
    std::vector<double> params;
};

class SoftmaxSimulatorBackend : public QubitBackend {
  public:
    explicit SoftmaxSimulatorBackend(BackendConfiguration config)
        : config_(std::move(config)), qubit_count_(0), active_qubits_(), operations_(),
          last_shots_(0), results_() {}

    void reset() override {
        qubit_count_ = 0;
        active_qubits_.clear();
        operations_.clear();
        results_.clear();
        last_shots_ = 0;
    }

    std::size_t allocate_qubit() override {
        active_qubits_.push_back(true);
        return qubit_count_++;
    }

    void release_qubit(std::size_t id) override {
        if (id >= active_qubits_.size() || !active_qubits_[id])
            throw std::out_of_range("qubit not allocated");
        active_qubits_[id] = false;
    }

    void apply_single_qubit_gate(const std::string &gate, std::size_t target,
                                 const std::vector<double> &params) override {
        (void)params;
        if (target >= active_qubits_.size() || !active_qubits_[target])
            throw std::out_of_range("target qubit not allocated");
        operations_.push_back({GateOperation::Kind::Single, gate, 0, target, params});
    }

    void apply_two_qubit_gate(const std::string &gate, std::size_t control, std::size_t target,
                              const std::vector<double> &params) override {
        (void)params;
        if (control >= active_qubits_.size() || target >= active_qubits_.size() ||
            !active_qubits_[control] || !active_qubits_[target])
            throw std::out_of_range("qubits not allocated for two-qubit gate");
        operations_.push_back({GateOperation::Kind::Two, gate, control, target, params});
    }

    void submit(std::size_t shots) override {
        last_shots_ = shots;
        (void)shots;
        // The simulator operates directly on analytical distributions; nothing to do here.
    }

    std::vector<double> retrieve_results() override { return results_; }

    std::vector<double> run_distribution_experiment(const std::vector<double> &weights,
                                                    std::size_t shots) override {
        auto probabilities = softmax(weights);
        results_.assign(probabilities.begin(), probabilities.end());

        if (config_.amplitude_output) {
            std::vector<double> amplitudes(results_.size(), 0.0);
            double norm = 0.0;
            for (std::size_t i = 0; i < probabilities.size(); ++i) {
                amplitudes[i] = std::sqrt(probabilities[i]);
                norm += amplitudes[i] * amplitudes[i];
            }
            if (norm > 0.0) {
                double inv = 1.0 / std::sqrt(norm);
                for (double &value : amplitudes)
                    value *= inv;
            }
            results_.swap(amplitudes);
            return results_;
        }

        if (results_.empty())
            return results_;

        if (shots == 0)
            return results_;

        std::vector<double> samples(results_.size(), 0.0);
        std::mt19937 rng(config_.seed);
        std::discrete_distribution<std::size_t> dist(results_.begin(), results_.end());
        for (std::size_t i = 0; i < shots; ++i)
            ++samples[dist(rng)];
        for (double &value : samples)
            value /= static_cast<double>(shots);
        results_.swap(samples);
        return results_;
    }

  private:
    BackendConfiguration config_;
    std::size_t qubit_count_;
    std::vector<bool> active_qubits_;
    std::vector<GateOperation> operations_;
    std::size_t last_shots_;
    std::vector<double> results_;
};

class OpenQasmHttpBackend : public QubitBackend {
  public:
    explicit OpenQasmHttpBackend(BackendConfiguration config)
        : config_(std::move(config)), qubit_count_(0), active_qubits_(), circuit_(),
          last_shots_(0), results_() {}

    void reset() override {
        qubit_count_ = 0;
        active_qubits_.clear();
        circuit_.clear();
        last_shots_ = 0;
        results_.clear();
    }

    std::size_t allocate_qubit() override {
        active_qubits_.push_back(true);
        return qubit_count_++;
    }

    void release_qubit(std::size_t id) override {
        if (id >= active_qubits_.size() || !active_qubits_[id])
            throw std::out_of_range("qubit not allocated");
        active_qubits_[id] = false;
    }

    void apply_single_qubit_gate(const std::string &gate, std::size_t target,
                                 const std::vector<double> &params) override {
        if (target >= active_qubits_.size() || !active_qubits_[target])
            throw std::out_of_range("target qubit not allocated");
        circuit_.push_back({GateOperation::Kind::Single, gate, 0, target, params});
    }

    void apply_two_qubit_gate(const std::string &gate, std::size_t control, std::size_t target,
                              const std::vector<double> &params) override {
        if (control >= active_qubits_.size() || target >= active_qubits_.size() ||
            !active_qubits_[control] || !active_qubits_[target])
            throw std::out_of_range("qubits not allocated for two-qubit gate");
        circuit_.push_back({GateOperation::Kind::Two, gate, control, target, params});
    }

    void submit(std::size_t shots) override {
        if (!config_.openqasm.submission_callback)
            throw std::runtime_error("no submission callback configured for OpenQASM backend");
        last_shots_ = shots;
        auto qasm = build_program_qasm();
        results_ = config_.openqasm.submission_callback(qasm, shots, config_.openqasm);
    }

    std::vector<double> retrieve_results() override { return results_; }

    std::vector<double> run_distribution_experiment(const std::vector<double> &weights,
                                                    std::size_t shots) override {
        last_shots_ = shots;

        if (config_.openqasm.submission_callback) {
            auto qasm = build_distribution_qasm(weights);
            results_ =
                config_.openqasm.submission_callback(qasm, shots, config_.openqasm);
            return results_;
        }

        auto probabilities = softmax(weights);
        if (probabilities.empty()) {
            results_.clear();
            return results_;
        }

        if (shots == 0) {
            results_ = probabilities;
            return results_;
        }

        std::vector<double> samples(probabilities.size(), 0.0);
        std::mt19937 rng(config_.seed);
        std::discrete_distribution<std::size_t> dist(probabilities.begin(), probabilities.end());
        for (std::size_t i = 0; i < shots; ++i)
            ++samples[dist(rng)];
        for (double &value : samples)
            value /= static_cast<double>(shots);
        results_.swap(samples);
        return results_;
    }

  private:
    std::string build_program_qasm() const {
        std::ostringstream oss;
        oss << "OPENQASM 2.0;\n";
        oss << "include \"qelib1.inc\";\n";
        oss << "qreg q[" << std::max<std::size_t>(qubit_count_, active_qubits_.size())
            << "];\n";
        oss << "creg c[" << std::max<std::size_t>(qubit_count_, active_qubits_.size())
            << "];\n";
        for (const auto &op : circuit_) {
            if (op.kind == GateOperation::Kind::Single) {
                oss << op.gate << " q[" << op.target << "]";
            } else {
                oss << op.gate << " q[" << op.control << "],q[" << op.target << "]";
            }
            if (!op.params.empty()) {
                oss << "(";
                for (std::size_t i = 0; i < op.params.size(); ++i) {
                    if (i > 0)
                        oss << ',';
                    oss << op.params[i];
                }
                oss << ")";
            }
            oss << ";\n";
        }
        oss << "measure q -> c;\n";
        return oss.str();
    }

    std::string build_distribution_qasm(const std::vector<double> &weights) const {
        std::ostringstream oss;
        oss << "OPENQASM 2.0;\n";
        oss << "// Distribution sampling request\n";
        oss << "// endpoint: " << config_.openqasm.endpoint << "\n";
        oss << "// backend: " << config_.openqasm.backend << "\n";
        oss << "// weights:";
        for (double value : weights)
            oss << ' ' << value;
        oss << "\n";
        oss << "// shots: " << last_shots_ << "\n";
        return oss.str();
    }

    BackendConfiguration config_;
    std::size_t qubit_count_;
    std::vector<bool> active_qubits_;
    std::vector<GateOperation> circuit_;
    std::size_t last_shots_;
    std::vector<double> results_;
};

} // namespace

std::unique_ptr<QubitBackend> make_backend(BackendKind kind, BackendConfiguration config) {
    switch (kind) {
    case BackendKind::CPU:
        config.amplitude_output = false;
        return std::make_unique<SoftmaxSimulatorBackend>(std::move(config));
    case BackendKind::QPU_SIM:
        config.amplitude_output = true;
        return std::make_unique<SoftmaxSimulatorBackend>(std::move(config));
    case BackendKind::OPENQASM_HTTP:
        config.amplitude_output = false;
        return std::make_unique<OpenQasmHttpBackend>(std::move(config));
    }
    throw std::invalid_argument("unknown backend kind");
}

} // namespace quantum
} // namespace qpp

