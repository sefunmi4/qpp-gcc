#include <cassert>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <cmath>

#include "qpp/backend/Factory.hpp"
#include "qpp/backend/LocalSimBackend.hpp"
#include "qpp/backend/QPUBackend.hpp"

using namespace qpp;

// RAII helper to manage environment variables
class ScopedEnv {
public:
    ScopedEnv(const char* name, const char* value) : name_{name} {
        const char* old = std::getenv(name);
        if (old) {
            had_old_ = true;
            old_ = old;
        }
        setenv(name_, value, 1);
    }
    ~ScopedEnv() {
        if (had_old_)
            setenv(name_, old_.c_str(), 1);
        else
            unsetenv(name_);
    }
private:
    const char* name_;
    std::string old_;
    bool had_old_ = false;
};

// Loopback backend that uses the simulator under the hood
class LoopbackQPUBackend : public QPUBackend {
public:
    bool available() const override { return true; }
    void loadCircuit(const std::string& circuit) override {
        QPUBackend::loadCircuit(circuit);
        sim_.loadCircuit(circuit);
    }
    RunResult run() override { return sim_.run(); }
private:
    LocalSimBackend sim_;
};

int main() {
    // BackendFactory selects hardware when available
    {
        ScopedEnv env{"QPU_PCI_DEVICE", "loopback"};
        auto backend = BackendFactory::create();
        assert(dynamic_cast<QPUBackend*>(backend.get()) &&
               "Factory should return QPU backend when hardware is present");
    }

    // BackendFactory falls back to simulator when no hardware is found
    {
        unsetenv("QPU_PCI_DEVICE");
        auto backend = BackendFactory::create();
        assert(dynamic_cast<LocalSimBackend*>(backend.get()) &&
               "Factory should return simulator when no hardware is present");
    }

    // Parity tests between mock hardware and simulator
    LoopbackQPUBackend hw;
    LocalSimBackend sim;

    // Single-qubit Hadamard
    std::string h_circuit = "h 0; measure 0;";
    hw.loadCircuit(h_circuit);
    sim.loadCircuit(h_circuit);
    RunResult hr = hw.run();
    RunResult sr = sim.run();
    assert(std::abs(hr.probabilities["0"] - sr.probabilities["0"]) < 1e-9);
    assert(std::abs(hr.probabilities["1"] - sr.probabilities["1"]) < 1e-9);

    // Two-qubit Grover search for state |11>
    std::string grover =
        "h 0; h 1; "
        "x 0; x 1; h 1; cx 0 1; h 1; x 0; x 1; "
        "h 0; h 1; x 0; x 1; h 1; cx 0 1; h 1; x 0; x 1; h 0; h 1; "
        "measure 0; measure 1;";
    hw.loadCircuit(grover);
    sim.loadCircuit(grover);
    hr = hw.run();
    sr = sim.run();
    assert(std::abs(hr.probabilities["11"] - sr.probabilities["11"]) < 1e-9);

    std::cout << "ok\n";
    return 0;
}

