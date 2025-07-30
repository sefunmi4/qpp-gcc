import subprocess
import tempfile
from pathlib import Path
import unittest

root_dir = Path(__file__).resolve().parents[1]
include_dir = root_dir / 'include'

class PeriodicityTests(unittest.TestCase):
    def compile_and_run(self, code, name):
        with tempfile.TemporaryDirectory() as tmp:
            src = Path(tmp) / f"{name}.cpp"
            src.write_text(code)
            exe = Path(tmp) / name
            subprocess.run(['g++', '-std=c++17', '-I', str(include_dir), str(src), '-o', str(exe)], check=True)
            result = subprocess.run([str(exe)], capture_output=True, text=True, check=True)
            return result.stdout.strip()

    def test_x_gate_periodicity(self):
        code = r"""
#include <iostream>
#include <complex>
#include \"qpp/qstruct.hpp\"
int main() {
    qpp::qclass qc(1);
    qc.apply_h(0);
    auto before = qc.data().amplitude;
    qc.apply_x(0);
    qc.apply_x(0);
    auto after = qc.data().amplitude;
    bool eq = std::abs(before[0] - after[0]) < 1e-9 &&
              std::abs(before[1] - after[1]) < 1e-9;
    std::cout << (eq ? "equal" : "not_equal") << '\n';
    return 0;
}
"""
        out = self.compile_and_run(code, 'xperiod')
        self.assertEqual(out, 'equal')

    def test_h_gate_periodicity(self):
        code = r"""
#include <iostream>
#include <complex>
#include \"qpp/qstruct.hpp\"
int main() {
    qpp::qclass qc(1);
    auto before = qc.data().amplitude;
    qc.apply_h(0);
    qc.apply_h(0);
    auto after = qc.data().amplitude;
    bool eq = std::abs(before[0] - after[0]) < 1e-9 &&
              std::abs(before[1] - after[1]) < 1e-9;
    std::cout << (eq ? "equal" : "not_equal") << '\n';
    return 0;
}
"""
        out = self.compile_and_run(code, 'hperiod')
        self.assertEqual(out, 'equal')

    def test_cx_gate_periodicity(self):
        code = r"""
#include <iostream>
#include <complex>
#include <vector>
#include \"qpp/qstruct.hpp\"
int main() {
    qpp::qclass qc(2);
    qc.apply_h(0);
    auto before = qc.data().amplitude;
    qc.apply_cx(0,1);
    qc.apply_cx(0,1);
    auto after = qc.data().amplitude;
    bool eq = true;
    for (std::size_t i = 0; i < before.size(); ++i)
        if (std::abs(before[i] - after[i]) >= 1e-9)
            eq = false;
    std::cout << (eq ? "equal" : "not_equal") << '\n';
    return 0;
}
"""
        out = self.compile_and_run(code, 'cxperiod')
        self.assertEqual(out, 'equal')

if __name__ == '__main__':
    unittest.main()
