#include <iostream>
#include <memory>
#include "qpp/api/Circuit.hpp"
#include "qpp/api/Program.hpp"
#include "qpp/backend/LocalSimBackend.hpp"
#include "qpp/hardware.hpp"
#include "qpp/qregister.hpp"
#include "qpp/cstruct.hpp"
#include "qpp/pbool.h"
#include "qpp/scheduler.hpp"
#include "qpp/sim/StateVector.hpp"
#include "qpp/sim/Gates.hpp"
#include "qpp/sim/Grover.hpp"
#include "qpp/sim/Characters.hpp"
#include "qpp/sim/Encodings.hpp"
#include "qpp/sim/PeriodFinding.hpp"

class QPPDemo {
public:
  void run() {
    using namespace qpp;
    std::cout << "== API / backend ==\n";
    api::Circuit c;
    c.allocateQubits(2);
    c.addGate("h", {0});
    c.addGate("cx", {0,1});
    c.shots = 256;
    api::Program prog(c, std::make_unique<LocalSimBackend>());
    auto res = prog.execute();
    std::cout << "Bell state ";
    for (auto& kv : res.counts) std::cout << kv.first << ":" << kv.second << " ";
    std::cout << "\n";

    std::cout << "== registers & structs ==\n";
    qregister qreg(1);
    qreg.data().apply_h(0);
    pbool m = qreg.data().measure(0);
    std::cout << "p(1)=" << m.probability() << " sample=" << static_cast<bool>(m) << "\n";
    cclass ctmp(3); ctmp.data().bits[1] = 1;
    cregister creg(2); creg[0] = 1;
    std::cout << "creg[0]=" << creg[0] << " cclass[1]=" << ctmp.data().bits[1] << "\n";

    std::cout << "== scheduler ==\n";
    Scheduler sch;
    sch.add([](){ std::cout << "low\n"; }, 1);
    sch.add([](){ std::cout << "high\n"; }, 2);
    sch.run();

    std::cout << "== simulation algorithms ==\n";
    sim::StateVector psi; psi.allocate(3);
    for(int q=0;q<3;++q) sim::apply_gate(psi, sim::H, q);
    sim::amplitude_amplification(psi, [](std::size_t i){return i==5;}, 1);
    std::cout << "Grover p[5]=" << psi.probabilities()[5] << "\n";
    auto primes = sim::prime_sieve_frequency(20);
    std::cout << "primes:"; for(auto p:primes) std::cout << " " << p; std::cout << "\n";
    sim::LFC lfc(2.0);
    std::cout << "encode=" << lfc.encode(3.5,0.1) << " decode=" << lfc.decodeValue(lfc.freq(3.5)) << "\n";
    std::cout << "period=" << sim::period_finding(2,7,6) << "\n";

    std::cout << "hardware_available=" << hardware_available() << "\n";
  }
};

int main() { QPPDemo demo; demo.run(); return 0; }
