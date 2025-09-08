#include <iostream>
#include "qpp/qstruct.hpp"

int main() {
    qpp::qclass reg(1); // one qubit register
    reg.apply_h(0);     // apply Hadamard
    reg.apply_x(0);     // apply Pauli-X
    const auto& st = reg.data();
    std::cout << "State amplitudes:\n";
    for (std::size_t i = 0; i < st.amplitude.size(); ++i)
        std::cout << i << ": " << st.amplitude[i] << '\n';
    return 0;
}
