#include <iostream>
#include "qpp/api/Program.hpp"

int main() {
    qpp::api::Circuit circ;
    circ.allocateQubits(2);
    circ.addGate("H", {0});
    circ.addGate("CX", {0, 1});
    circ.addGate("measure", {0, 1});

    qpp::LocalSimBackend sim;
    qpp::api::Program prog(circ, nullptr, &sim);
    auto result = prog.execute();

    for (const auto& [bits, prob] : result.probabilities)
        std::cout << bits << ": " << prob << '\n';
    return 0;
}
