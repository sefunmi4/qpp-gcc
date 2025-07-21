#include <iostream>
#include "qpp/pbool.h"

int main() {
    qpp::pbool a{0.6};
    qpp::pbool b{0.7};
    qpp::pbool c = a && b;
    std::cout << "P(a=true)=" << a.probability()
              << "\nP(b=true)=" << b.probability()
              << "\nP(a&&b=true)=" << c.probability() << '\n';
    return static_cast<bool>(c) ? 0 : 1;
}
