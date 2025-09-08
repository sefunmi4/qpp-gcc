#include <chrono>
#include <iostream>
#include <memory>

#include "qpp/api/Program.hpp"
#include "qpp/backend/LocalSimBackend.hpp"
#include "qpp/backend/QPUBackend.hpp"

int main() {
    using clock = std::chrono::high_resolution_clock;

    qpp::api::Circuit circ;
    circ.allocateQubits(2);
    circ.addGate("H", {0});
    circ.addGate("CX", {0, 1});
    circ.addGate("measure", {0, 1});
    circ.shots = 1024;

    auto devices = qpp::QPUBackend::discoverDevices();
    std::unique_ptr<qpp::Backend> backend;
    std::string backend_name;

    if (!devices.empty()) {
        std::cout << "Using hardware device: " << devices.front() << "\n";
        backend = std::make_unique<qpp::QPUBackend>();
        backend_name = "QPUBackend";
    } else {
        std::cout << "No hardware devices found. Falling back to LocalSimBackend.\n";
        backend = std::make_unique<qpp::LocalSimBackend>();
        backend_name = "LocalSimBackend";
    }

    qpp::api::Program hw_prog(circ, std::move(backend));
    auto start_hw = clock::now();
    auto hw_result = hw_prog.execute();
    auto end_hw = clock::now();
    double hw_time = std::chrono::duration<double>(end_hw - start_hw).count();
    double hw_throughput = circ.shots / hw_time;

    qpp::LocalSimBackend sim;
    qpp::api::Program sim_prog(circ, nullptr, &sim);
    auto start_sim = clock::now();
    auto sim_result = sim_prog.execute();
    auto end_sim = clock::now();
    double sim_time = std::chrono::duration<double>(end_sim - start_sim).count();
    double sim_throughput = circ.shots / sim_time;

    std::cout << backend_name << " time: " << hw_time
              << " s, throughput: " << hw_throughput << " shots/s\n";
    std::cout << "LocalSimBackend time: " << sim_time
              << " s, throughput: " << sim_throughput << " shots/s\n";

    return 0;
}
