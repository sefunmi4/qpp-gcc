import subprocess
import tempfile
from pathlib import Path
import unittest

root_dir = Path(__file__).resolve().parents[2]
include_dir = root_dir / 'include'

class SimAlgorithmTests(unittest.TestCase):
    def compile_and_run(self, code: str, name: str) -> str:
        with tempfile.TemporaryDirectory() as tmp:
            src = Path(tmp) / f"{name}.cpp"
            src.write_text(code)
            exe = Path(tmp) / name
            subprocess.run(
                [
                    'g++',
                    '-std=c++17',
                    '-I', str(include_dir),
                    '-I', str(root_dir),
                    str(src),
                    '-o', str(exe),
                ],
                check=True,
            )
            result = subprocess.run([str(exe)], capture_output=True, text=True, check=True)
            return result.stdout.strip()

    def test_gate_identities(self):
        code = r"""#include <iostream>
#include <random>
#include <complex>
#include "qpp/sim/StateVector.hpp"
#include "qpp/sim/Gates.hpp"
int main() {
    std::mt19937 gen(42);
    std::uniform_real_distribution<qpp::sim::real_t> dist(-1.0,1.0);
    qpp::sim::StateVector psi;
    psi.allocate(1);
    psi.data[0] = {dist(gen), dist(gen)};
    psi.data[1] = {dist(gen), dist(gen)};
    psi.normalize();
    auto before = psi.data;
    qpp::sim::apply_gate(psi, qpp::sim::H, 0);
    qpp::sim::apply_gate(psi, qpp::sim::H, 0);
    bool h_ok = std::abs(psi.data[0] - before[0]) < 1e-9 &&
                std::abs(psi.data[1] - before[1]) < 1e-9;
    psi.data = before;
    qpp::sim::StateVector zpsi = psi;
    qpp::sim::apply_gate(zpsi, qpp::sim::Z, 0);
    qpp::sim::StateVector xzxpsi = psi;
    qpp::sim::apply_gate(xzxpsi, qpp::sim::X, 0);
    qpp::sim::apply_gate(xzxpsi, qpp::sim::Z, 0);
    qpp::sim::apply_gate(xzxpsi, qpp::sim::X, 0);
    bool xzx_ok = std::abs(xzxpsi.data[0] + zpsi.data[0]) < 1e-9 &&
                  std::abs(xzxpsi.data[1] + zpsi.data[1]) < 1e-9;
    std::cout << ((h_ok && xzx_ok) ? "ok" : "fail") << '\n';
    return 0;
}
"""
        out = self.compile_and_run(code, 'gate_ids')
        self.assertEqual(out, 'ok')

    def test_bell_state(self):
        code = r"""#include <iostream>
#include <cmath>
#include "qpp/sim/StateVector.hpp"
#include "qpp/sim/Gates.hpp"
int main(){
    qpp::sim::StateVector psi;
    psi.allocate(2);
    qpp::sim::apply_gate(psi, qpp::sim::H, 0);
    qpp::sim::apply_cx(psi,0,1);
    double p00 = psi.probability(0);
    double p11 = psi.probability(3);
    double p01 = psi.probability(1);
    double p10 = psi.probability(2);
    bool bell = std::abs(p00-0.5) < 1e-9 && std::abs(p11-0.5) < 1e-9 &&
                p01 < 1e-9 && p10 < 1e-9;
    std::cout << (bell ? "ok" : "fail") << '\n';
    return 0;
}
"""
        out = self.compile_and_run(code, 'bell')
        self.assertEqual(out, 'ok')

    def test_amplitude_amplification(self):
        code = r"""#include <iostream>
#include <cmath>
#include "qpp/sim/StateVector.hpp"
#include "qpp/sim/Gates.hpp"
#include "qpp/sim/Grover.hpp"
int main(){
    qpp::sim::StateVector psi;
    psi.allocate(3);
    for(int q=0;q<3;++q) qpp::sim::apply_gate(psi, qpp::sim::H, q);
    qpp::sim::amplitude_amplification(psi, [](std::size_t i){return i==3;}, 1);
    auto probs = psi.probabilities();
    bool amplified = probs[3] > 0.5;
    std::cout << (amplified ? "ok" : "fail") << '\n';
    return 0;
}
"""
        out = self.compile_and_run(code, 'grover')
        self.assertEqual(out, 'ok')

    def test_proj_mod_eq_and_sieve(self):
        code = r"""#include <iostream>
#include <vector>
#include "qpp/sim/Characters.hpp"
int main(){
    bool eq1 = qpp::sim::proj_mod_eq(10,22,12);
    bool eq2 = !qpp::sim::proj_mod_eq(10,23,12);
    auto primes = qpp::sim::prime_sieve_frequency(20);
    bool p_ok = primes == std::vector<int>{2,3,5,7,11,13,17,19};
    std::cout << ((eq1 && eq2 && p_ok) ? "ok" : "fail") << '\n';
    return 0;
}
"""
        out = self.compile_and_run(code, 'chars')
        self.assertEqual(out, 'ok')

    def test_encoding_roundtrip(self):
        code = r"""#include <iostream>
#include <cmath>
#include "qpp/sim/Encodings.hpp"
int main(){
    qpp::sim::LFC lfc(2.0);
    double v1 = 3.5;
    double d1 = lfc.decodeValue(lfc.freq(v1));
    qpp::sim::LogFC logfc(1.5);
    double v2 = 4.2;
    double d2 = logfc.decodeValue(logfc.freq(v2));
    bool ok = std::abs(d1 - v1) < 1e-9 && std::abs(d2 - v2) < 1e-9;
    std::cout << (ok ? "ok" : "fail") << '\n';
    return 0;
}
"""
        out = self.compile_and_run(code, 'encodings')
        self.assertEqual(out, 'ok')

    def test_period_finding(self):
        code = r"""#include <iostream>
#include "qpp/sim/PeriodFinding.hpp"
int main(){
    std::size_t r = qpp::sim::period_finding(2,7,6);
    std::cout << (r == 3 ? "ok" : "fail") << '\n';
    return 0;
}
"""
        out = self.compile_and_run(code, 'period')
        self.assertEqual(out, 'ok')

if __name__ == '__main__':
    unittest.main()
