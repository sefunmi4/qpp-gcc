import subprocess
import tempfile
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
INCLUDE = ROOT / "include"

CODE = Path(ROOT / "examples" / "master_demo.cpp").read_text()

class MasterDemoTest(unittest.TestCase):
    def test_demo_runs(self):
        with tempfile.TemporaryDirectory() as tmp:
            src = Path(tmp) / "demo.cpp"
            exe = Path(tmp) / "demo"
            src.write_text(CODE)
            subprocess.run(
                [
                    "g++",
                    "-std=c++17",
                    "-I",
                    str(INCLUDE),
                    "-I",
                    str(ROOT),
                    str(src),
                    str(ROOT / "qpp" / "backend" / "LocalSimBackend.cpp"),
                    "-o",
                    str(exe),
                ],
                check=True,
            )
            out = subprocess.run([str(exe)], capture_output=True, text=True, check=True).stdout
            self.assertIn("Bell state", out)
            self.assertIn("Grover", out)
            self.assertIn("primes:", out)

if __name__ == "__main__":
    unittest.main()
