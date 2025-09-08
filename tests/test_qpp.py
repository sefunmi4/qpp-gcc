import subprocess
import json
import tempfile
from pathlib import Path
import unittest

root_dir = Path(__file__).resolve().parents[1]
include_dir = root_dir / 'include'

class QppTests(unittest.TestCase):
    def compile_and_run(self, source, exe_name):
        with tempfile.TemporaryDirectory() as tmp:
            exe = Path(tmp) / exe_name
            subprocess.run(['g++', '-I', str(include_dir), source, '-o', str(exe)], check=True)
            result = subprocess.run([str(exe)], capture_output=True, text=True)
            return result.stdout

    def test_qstruct_demo(self):
        out = self.compile_and_run(str(root_dir / 'examples/qstruct_demo.cpp'), 'qs')
        self.assertIn('0: (0.707107', out)
        self.assertIn('1: (0.707107', out)

    def test_pbool_demo(self):
        out = self.compile_and_run(str(root_dir / 'examples/pbool_demo.cpp'), 'pb')
        self.assertIn('P(a=true)=0.6', out)
        self.assertIn('P(a&&b=true)=0.42', out)

    def test_scheduler_demo(self):
        out = self.compile_and_run(str(root_dir / 'examples/scheduler_demo.cpp'), 'sch')
        self.assertIn('H 0', out)
        self.assertIn('X 0', out)

    def test_register_import_export(self):
        code = r"""
#include <iostream>
#include "qpp/qregister.hpp"
int main() {
    qpp::qregister qr(1);
    qr.data().apply_x(0);
    auto st = qr.export_state();
    qr.data().apply_x(0);
    qr.import_state(st);
    const auto& amp = qr.data().data().amplitude;
    std::cout << amp[0] << ' ' << amp[1] << '\n';
    return 0;
}
"""
        with tempfile.TemporaryDirectory() as tmp:
            src = Path(tmp) / 'reg.cpp'
            src.write_text(code)
            exe = Path(tmp) / 'reg'
            subprocess.run(['g++', '-I', str(include_dir), str(src), '-o', str(exe)], check=True)
            result = subprocess.run([str(exe)], check=True, capture_output=True, text=True)
            self.assertIn('(0,0)', result.stdout)
            self.assertIn('(1,0)', result.stdout)

    def test_parser_ir(self):
        result = subprocess.run(['python3', str(root_dir / 'contrib/qpp_parse_ir.py'), str(root_dir / 'examples/qpp_parse_example.qpp')], capture_output=True, text=True, check=True)
        data = json.loads(result.stdout)
        self.assertEqual(data['structs'][0]['name'], 'Token')
        self.assertEqual(data['tasks'][0]['target'], 'QPU')
        self.assertEqual(len(data['qasm']), 1)

    def test_measure_pbool(self):
        code = r"""
#include <iostream>
#include "qpp/qstruct.hpp"
int main() {
    qpp::qclass qc(1);
    qc.apply_h(0);
    auto p = qc.measure(0);
    std::cout << p.probability() << '\n';
    return 0;
}
"""
        with tempfile.TemporaryDirectory() as tmp:
            src = Path(tmp) / 'meas.cpp'
            src.write_text(code)
            exe = Path(tmp) / 'meas'
            subprocess.run(['g++', '-I', str(include_dir), str(src), '-o', str(exe)], check=True)
            result = subprocess.run([str(exe)], check=True, capture_output=True, text=True)
            self.assertTrue(result.stdout.startswith('0.5'))

    def test_program_execute_local(self):
        code = r"""
#include <iostream>
#include "qpp/api/Program.hpp"
int main() {
    qpp::api::Circuit circ;
    circ.allocateQubits(1);
    circ.addGate("h", {0});
    circ.addGate("measure", {0});
    qpp::LocalSimBackend sim;
    qpp::api::Program prog(circ, nullptr, &sim);
    auto res = prog.execute();
    std::cout << res.probabilities["0"] << ' ' << res.probabilities["1"] << '\n';
    return 0;
}
"""
        with tempfile.TemporaryDirectory() as tmp:
            src = Path(tmp) / 'prog.cpp'
            src.write_text(code)
            exe = Path(tmp) / 'prog'
            subprocess.run(['g++', '-I', str(include_dir), str(src), str(root_dir / 'qpp/backend/LocalSimBackend.cpp'), '-o', str(exe)], check=True)
            result = subprocess.run([str(exe)], check=True, capture_output=True, text=True)
            self.assertIn('0.5', result.stdout)

    def test_features_file(self):
        text = (root_dir / 'features.yaml').read_text()
        self.assertIn('circuit_simplification', text)
        self.assertIn('gate_fusion', text)
        self.assertIn('explain_directive', text)

if __name__ == '__main__':
    unittest.main()
