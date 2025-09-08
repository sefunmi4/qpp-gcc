#include <iostream>
#include "qpp/pbool.h"
#include "qpp/hardware.hpp"

int main() {
    qpp::pbool a{0.6};
    qpp::pbool b{0.7};
    qpp::pbool c = a && b;
    std::cout << "P(a=true)=" << a.probability()
              << "\nP(b=true)=" << b.probability()
              << "\nP(a&&b=true)=" << c.probability() << '\n';
    if (!qpp::hardware_available())
        std::cout << "Quantum hardware unavailable; using classical RNG\n";
    std::cout << "Sampled value: " << c.sample() << '\n';
    return static_cast<bool>(c) ? 0 : 1;
}
