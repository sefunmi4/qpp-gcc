import importlib
import sys
import tempfile
from pathlib import Path
import unittest

sys.path.append(str(Path(__file__).resolve().parents[1]))
from hardware_api import HardwareAPI


class HardwareAPITests(unittest.TestCase):
    def test_qiskit_simulator_by_default(self):
        if importlib.util.find_spec("qiskit") is None:
            self.skipTest("qiskit not installed")
        api = HardwareAPI("qiskit")
        backend = api.connect()
        self.assertIn("simulator", backend.name().lower())

    def test_cirq_simulator_by_default(self):
        if importlib.util.find_spec("cirq") is None:
            self.skipTest("cirq not installed")
        import cirq
        api = HardwareAPI("cirq")
        backend = api.connect()
        self.assertIsInstance(backend, cirq.Simulator)

    def test_run_qasm_on_qiskit(self):
        if importlib.util.find_spec("qiskit") is None:
            self.skipTest("qiskit not installed")
        api = HardwareAPI("qiskit")
        qasm = (
            "OPENQASM 2.0; include \"qelib1.inc\"; "
            "qreg q[1]; creg c[1]; h q[0]; measure q[0] -> c[0];"
        )
        result = api.run_qasm(qasm, shots=1)
        # Result counts should sum to the number of shots
        self.assertEqual(sum(result.get_counts().values()), 1)

    def test_run_qasm_on_cirq(self):
        if importlib.util.find_spec("cirq") is None:
            self.skipTest("cirq not installed")
        api = HardwareAPI("cirq")
        qasm = (
            "OPENQASM 2.0; include \"qelib1.inc\"; "
            "qreg q[1]; creg c[1]; h q[0]; measure q[0] -> c[0];"
        )
        result = api.run_qasm(qasm, shots=1)
        # The simulator should produce a measurement record
        self.assertTrue(result.measurements)

    def test_select_backend_qiskit(self):
        if importlib.util.find_spec("qiskit") is None:
            self.skipTest("qiskit not installed")
        api = HardwareAPI("qiskit")
        api.select_backend("statevector_simulator")
        backend = api.connect()
        self.assertIn("statevector", backend.name())

    def test_select_backend_cirq(self):
        if importlib.util.find_spec("cirq") is None:
            self.skipTest("cirq not installed")
        import cirq
        api = HardwareAPI("cirq")
        api.select_backend("density_matrix")
        backend = api.connect()
        self.assertIsInstance(backend, cirq.DensityMatrixSimulator)

    def test_run_source_qiskit(self):
        if importlib.util.find_spec("qiskit") is None:
            self.skipTest("qiskit not installed")
        api = HardwareAPI("qiskit")
        code = (
            "int main() {\n"
            "Circuit circuit;\n"
            "circuit.push_back(H(0));\n"
            "circuit.push_back(CX(0,1));\n"
            "circuit.push_back(Measure(0,1));\n"
            "}\n"
        )
        with tempfile.TemporaryDirectory() as tmp:
            src = Path(tmp) / "prog.qpp"
            src.write_text(code)
            result = api.run_source(str(src), shots=1)[0]
        self.assertEqual(sum(result.get_counts().values()), 1)

    def test_run_source_cirq(self):
        if importlib.util.find_spec("cirq") is None:
            self.skipTest("cirq not installed")
        api = HardwareAPI("cirq")
        code = (
            "int main() {\n"
            "Circuit circuit;\n"
            "circuit.push_back(H(0));\n"
            "circuit.push_back(CX(0,1));\n"
            "circuit.push_back(Measure(0,1));\n"
            "}\n"
        )
        with tempfile.TemporaryDirectory() as tmp:
            src = Path(tmp) / "prog.qpp"
            src.write_text(code)
            result = api.run_source(str(src), shots=1)[0]
        self.assertTrue(result.measurements)


if __name__ == "__main__":
    unittest.main()
