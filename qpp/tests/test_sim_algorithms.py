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

    def test_qarray_quantum_hooks(self):
        code = r"""#include <cmath>
#include <iostream>
#include <utility>
#include <vector>
#include "qpp/qarray"
#include "qpp/backend/notifications.hpp"

int main(){
    using arr_t = std::qarray<int, 4>;
    arr_t base{1, 2, 3, 4};
    bool init_ok = base[0] == 1 && base[3] == 4 && base.size() == arr_t::extent();

    std::vector<qpp::backend::Notification> notifications;
    qpp::backend::set_notification_sink(
        [&](const qpp::backend::Notification& n){ notifications.push_back(n); });

    arr_t copy = base;
    arr_t moved = std::move(base);

    bool clone_notified = !notifications.empty() &&
        notifications.front().kind == qpp::backend::NotificationKind::Clone &&
        notifications.front().size == copy.size();

    double uniform_prob = copy.probability_of(2);
    bool uniform_ok = std::abs(uniform_prob - 0.25) < 1e-9;

    int measured = copy.measure(2);
    double collapsed_prob = copy.probability_of(2);
    bool measurement_notified = !notifications.empty() &&
        notifications.back().kind == qpp::backend::NotificationKind::Measurement &&
        notifications.back().index == 2 &&
        std::abs(notifications.back().probability - uniform_prob) < 1e-9;
    bool measurement_ok = measured == 3 && collapsed_prob > 0.999;

    bool move_state_ok = std::abs(moved.probability_of(1) - 0.25) < 1e-9;

    qpp::backend::set_notification_sink({});
    bool ok = init_ok && clone_notified && uniform_ok && measurement_notified &&
              measurement_ok && move_state_ok;
    std::cout << (ok ? "ok" : "fail") << '\n';
    return 0;
}
"""
        out = self.compile_and_run(code, 'qarray_hooks')
        self.assertEqual(out, 'ok')

    def test_qvector_superposition_and_measurement(self):
        code = r"""#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <vector>
#include "qpp/backend/notifications.hpp"
#include "qpp/qvector"

int main(){
    using qpp::backend::Notification;
    using qpp::backend::NotificationKind;
    using qpp::backend::set_notification_sink;

    std::vector<Notification> events;
    set_notification_sink([&](const Notification& n){ events.push_back(n); });

    qpp::qvector<int> values{1, 2, 3};
    auto probs = values.probabilities();
    if (probs.size() != 3)
        return (std::cout << "fail\n", 0);
    for (double p : probs)
        if (std::abs(p - 1.0 / 3.0) > 1e-6)
            return (std::cout << "fail\n", 0);

    auto copy = values;
    if (events.empty() || events.back().kind != NotificationKind::Clone)
        return (std::cout << "fail\n", 0);

    values.pop_back();
    probs = values.probabilities();
    if (probs.size() != 2)
        return (std::cout << "fail\n", 0);
    if (std::abs(probs[0] - 0.5) > 1e-6 || std::abs(probs[1] - 0.5) > 1e-6)
        return (std::cout << "fail\n", 0);

    const int measured = values.measure(1);
    if (measured != 2)
        return (std::cout << "fail\n", 0);

    probs = values.probabilities();
    if (probs.size() != 2)
        return (std::cout << "fail\n", 0);
    if (probs[0] > 1e-9 || std::abs(probs[1] - 1.0) > 1e-9)
        return (std::cout << "fail\n", 0);

    if (events.size() < 2)
        return (std::cout << "fail\n", 0);
    const auto& measure_event = events.back();
    if (measure_event.kind != NotificationKind::Measurement || measure_event.index != 1)
        return (std::cout << "fail\n", 0);
    if (std::abs(measure_event.probability - 0.5) > 1e-6)
        return (std::cout << "fail\n", 0);

    const int total = std::accumulate(values.begin(), values.end(), 0);
    if (total != 3)
        return (std::cout << "fail\n", 0);

    set_notification_sink({});
    std::cout << "ok\n";
    return 0;
}
"""
        out = self.compile_and_run(code, 'qvector_superposition')
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
