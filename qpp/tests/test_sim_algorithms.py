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

    def test_qint_measurements_follow_sine_distribution(self):
        code = r"""#include <array>
#include <cmath>
#include <iostream>
#include "qpp/qint"
int main(){
    qint state;
    double frequency = 0.73;
    state.prepare_normalized_amplitudes(frequency);
    auto stats = state.measure_register(4096);
    std::array<double,4> expected{};
    double total = 0.0;
    for (std::size_t axis = 0; axis < expected.size(); ++axis) {
        double sample = std::sin(frequency * static_cast<double>(axis));
        expected[axis] = sample * sample;
        total += expected[axis];
    }
    if (total == 0.0)
        total = 1.0;
    bool ok = true;
    for (std::size_t axis = 0; axis < expected.size(); ++axis) {
        double target = expected[axis] / total;
        double observed_prob = 0.0;
        auto pit = stats[axis].probabilities.find(1);
        if (pit != stats[axis].probabilities.end())
            observed_prob = pit->second;
        double observed_freq = 0.0;
        auto cit = stats[axis].counts.find(1);
        if (cit != stats[axis].counts.end())
            observed_freq = static_cast<double>(cit->second) / 4096.0;
        if (!stats[axis].collapsed_value.has_value())
            ok = false;
        if (std::abs(observed_prob - target) > 0.05 ||
            std::abs(observed_freq - target) > 0.08) {
            ok = false;
            break;
        }
    }
    std::cout << (ok ? "ok" : "fail") << '\n';
    return 0;
}
"""
        out = self.compile_and_run(code, 'qint_sine_measure')
        self.assertEqual(out, 'ok')

    def test_qint_measurement_classical_fallback(self):
        code = r"""#include <array>
#include <cmath>
#include <iostream>
#include "qpp/qint"
int main(){
    qint state(-1, 1, -1, 1);
    auto stats = state.measure_register();
    const std::array<int,4> expected{{-1, 1, -1, 1}};
    bool ok = true;
    for (std::size_t axis = 0; axis < expected.size(); ++axis) {
        if (!stats[axis].collapsed_value.has_value() ||
            stats[axis].collapsed_value.value() != expected[axis]) {
            ok = false;
            break;
        }
        auto pit = stats[axis].probabilities.find(expected[axis]);
        if (pit == stats[axis].probabilities.end() || std::abs(pit->second - 1.0) > 1e-9) {
            ok = false;
            break;
        }
        if (!stats[axis].counts.empty()) {
            ok = false;
            break;
        }
    }
    std::cout << (ok ? "ok" : "fail") << '\n';
    return 0;
}
"""
        out = self.compile_and_run(code, 'qint_classical_measure')
        self.assertEqual(out, 'ok')

    def test_qarray_initialization_and_state(self):
        code = r"""#include <array>
#include <cmath>
#include <iostream>
#include "qpp/qarray"

int main() {
    const std::array<int, 3> classical{{1, 2, 3}};
    std::qarray<int, 3> quantum(classical);
    const auto& state = quantum.quantum_state();
    if (state.amplitude.size() != 4)
        return 1;
    const double normaliser = std::sqrt(14.0);
    bool ok = true;
    ok &= std::abs(state.amplitude[0].real() - (1.0 / normaliser)) < 1e-9;
    ok &= std::abs(state.amplitude[1].real() - (2.0 / normaliser)) < 1e-9;
    ok &= std::abs(state.amplitude[2].real() - (3.0 / normaliser)) < 1e-9;
    ok &= std::abs(state.amplitude[3].real()) < 1e-12;
    const auto before = quantum.measurement_epoch();
    quantum.mark_measured();
    ok &= quantum.measurement_epoch() == before + 1;
    std::cout << (ok ? "ok" : "fail") << '\n';
    return ok ? 0 : 1;
}
"""
        out = self.compile_and_run(code, 'qarray_init_state')
        self.assertEqual(out, 'ok')

    def test_qarray_copy_move_entanglement(self):
        code = r"""#include <cmath>
#include <iostream>
#include "qpp/qarray"

int main() {
    std::qarray<int, 3> source{1, 2, 3};
    auto baseline_state = source.quantum_state();
    const double normaliser = std::sqrt(14.0);
    bool ok = std::abs(baseline_state.amplitude[0].real() - (1.0 / normaliser)) < 1e-9;

    std::qarray<int, 3> copy = source;
    copy[0] = 42;
    const auto copy_state = copy.quantum_state();
    ok &= std::abs(copy_state.amplitude[0].real() - (42.0 / std::sqrt(42.0 * 42.0 + 2.0 * 2.0 + 3.0 * 3.0))) < 1e-9;
    const auto source_state = source.quantum_state();
    ok &= std::abs(source_state.amplitude[0].real() - (1.0 / normaliser)) < 1e-9;

    std::qarray<int, 3> moved = std::move(copy);
    moved[1] = -1;
    const auto moved_state = moved.quantum_state();
    ok &= moved_state.amplitude.size() == 4;

    const auto before_measure = source.measurement_epoch();
    source.mark_measured();
    ok &= source.measurement_epoch() == before_measure + 1;
    std::cout << (ok ? "ok" : "fail") << '\n';
    return ok ? 0 : 1;
}
"""
        out = self.compile_and_run(code, 'qarray_copy_move')
        self.assertEqual(out, 'ok')

    def test_qarray_matches_qvector_state(self):
        code = r"""#include <cmath>
#include <iostream>
#include "qpp/qarray"
#include "qpp/qvector"

int main() {
    std::qarray<int, 3> quantum_array{2, 1, 0};
    std::qvector<int> quantum_vector{2, 1, 0};
    const auto& array_state = quantum_array.quantum_state();
    const auto& vector_state = quantum_vector.quantum_state();
    if (array_state.amplitude.size() != vector_state.amplitude.size())
        return 1;
    bool ok = true;
    for (std::size_t i = 0; i < array_state.amplitude.size(); ++i) {
        ok &= std::abs(array_state.amplitude[i].real() - vector_state.amplitude[i].real()) < 1e-9;
    }
    std::cout << (ok ? "ok" : "fail") << '\n';
    return ok ? 0 : 1;
}
"""
        out = self.compile_and_run(code, 'qarray_qvector_sync')
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
