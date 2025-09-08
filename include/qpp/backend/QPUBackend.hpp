#ifndef QPP_BACKEND_QPUBACKEND_HPP
#define QPP_BACKEND_QPUBACKEND_HPP

#include <functional>
#include <random>
#include <string>
#include <vector>

#include "Backend.hpp"

namespace qpp {

/// Backend that interfaces with a remote or physical QPU.
///
/// The implementation intentionally keeps the hardware interaction
/// lightweight so that the backend can be compiled even when no vendor SDK
/// is present.  Device discovery attempts to locate hardware via PCI, USB or
/// a network API.  The connection routine also accepts an optional
/// authentication token.
class QPUBackend : public Backend {
public:
    using NoiseModel = std::function<void(RunResult&)>;

    QPUBackend();

    /// Returns true if a device has been discovered and connected.
    bool available() const override { return connected_; }

    /// Load a generic circuit description. The circuit is converted to a list
    /// of vendor specific instructions.
    void loadCircuit(const std::string& circuit) override;

    /// Execute the previously loaded circuit.
    RunResult run() override;

    /// Discover available devices across different interconnects.
    static std::vector<std::string> discoverDevices();

    /// Connect to a specific device providing an optional authentication token.
    void connect(const std::string& device, const std::string& token = {});

    /// Install a custom noise model that can post-process run results.
    void setNoiseModel(NoiseModel model) { noiseModel_ = std::move(model); }

private:
    struct Instruction {
        std::string opcode;
        std::vector<int> args;
    };

    static std::string mapGate(const std::string& gate);

    std::vector<Instruction> program_;
    std::vector<int> measuredQubits_;
    unsigned shots_ = 0;
    bool connected_ = false;
    std::string device_;
    std::string authToken_;
    NoiseModel noiseModel_{};
    std::mt19937 rng_{std::random_device{}()};
};

} // namespace qpp

#endif // QPP_BACKEND_QPUBACKEND_HPP

