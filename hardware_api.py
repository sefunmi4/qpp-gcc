"""Hardware API connectors for Qiskit and Cirq.

This module exposes a minimal interface for obtaining either
Qiskit or Cirq backends.  Credentials can be supplied via
:func:`add_credentials`. The previous :func:`add_credientials`
spelling is preserved as an alias for backward compatibility.
If credentials are not provided the
connectors fall back to the frameworks' default simulators.

Example
-------

>>> api = HardwareAPI('qiskit')
>>> api.add_credentials('qiskit', token='MYTOKEN')
>>> backend = api.connect()

"""
from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Dict, List

try:  # Optional imports; the package may not be installed.
    import qiskit
except Exception:  # pragma: no cover - the tests handle absence
    qiskit = None  # type: ignore

try:
    import cirq
except Exception:  # pragma: no cover - the tests handle absence
    cirq = None  # type: ignore


@dataclass
class HardwareAPI:
    """Connect to a quantum hardware provider.

    Parameters
    ----------
    target:
        ``"qiskit"`` or ``"cirq"``. Determines which backend
        library to use when :func:`connect` is invoked.
    """

    target: str
    credentials: Dict[str, Dict[str, Any]] = field(default_factory=dict)
    backend: str | None = None

    def add_credentials(self, provider: str, **kwargs: Any) -> None:
        """Store API credentials for a provider.

        Parameters
        ----------
        provider:
            ``"qiskit"`` or ``"cirq"``.
        kwargs:
            Keyword arguments containing credential information such as
            tokens or project identifiers. These are passed directly to
            the respective framework when establishing a connection.
        """

        self.credentials[provider.lower()] = kwargs

    # Backward compatibility for prior misspelling
    add_credientials = add_credentials

    def select_backend(self, name: str) -> None:
        """Select the backend or QPU to run programs on."""

        self.backend = name

    def connect(self) -> Any:
        """Return a backend instance for the configured target.

        If no credentials were supplied for the selected target the
        function returns a local simulator.  Supplying credentials
        enables access to remote hardware when supported by the
        framework.
        """

        tgt = self.target.lower()
        if tgt == "qiskit":
            if qiskit is None:  # pragma: no cover - runtime check
                raise ImportError("qiskit is not installed")
            creds = self.credentials.get("qiskit")
            if creds and "token" in creds:
                qiskit.IBMQ.save_account(creds["token"], overwrite=True)
                provider = qiskit.IBMQ.load_account()
                backend_name = self.backend or creds.get("backend", "ibmq_qasm_simulator")
                return provider.get_backend(backend_name)
            else:
                backend_name = self.backend or "qasm_simulator"
                return qiskit.Aer.get_backend(backend_name)
        elif tgt == "cirq":
            if cirq is None:  # pragma: no cover - runtime check
                raise ImportError("cirq is not installed")
            creds = self.credentials.get("cirq")
            if creds and "api_key" in creds:
                engine = cirq.google.Engine(project_id=creds.get("project_id"))
                processor = self.backend or creds.get("processor_id", "simulator")
                return engine.get_processor(processor)
            else:
                if self.backend == "density_matrix":
                    return cirq.DensityMatrixSimulator()
                return cirq.Simulator()
        else:
            raise ValueError(f"Unknown target '{self.target}'")

    def run_qasm(self, qasm: str, shots: int = 1024) -> Any:
        """Execute an OpenQASM program on the selected backend.

        The QASM string can be produced by the Q++/C++ toolchain and
        submitted here for execution on either simulator or hardware.

        Parameters
        ----------
        qasm:
            An OpenQASM program string.
        shots:
            Number of repetitions to execute, passed through to the
            underlying framework.
        """

        backend = self.connect()
        tgt = self.target.lower()
        if tgt == "qiskit":
            if qiskit is None:  # pragma: no cover - runtime check
                raise ImportError("qiskit is not installed")
            qc = qiskit.QuantumCircuit.from_qasm_str(qasm)
            job = qiskit.execute(qc, backend=backend, shots=shots)
            return job.result()
        elif tgt == "cirq":
            if cirq is None:  # pragma: no cover - runtime check
                raise ImportError("cirq is not installed")
            from cirq.contrib.qasm_import import circuit_from_qasm

            circuit = circuit_from_qasm(qasm)
            return backend.run(circuit, repetitions=shots)
        else:
            raise ValueError(f"Unknown target '{self.target}'")

    def run_source(self, source_path: str, shots: int = 1024) -> List[Any]:
        """Compile Q++/C++ source and execute on the selected backend.

        Parameters
        ----------
        source_path:
            Path to a source file containing either ``__qasm`` blocks or
            ``circuit.push_back`` statements.
        shots:
            Number of repetitions to execute for each extracted program.
        """

        from contrib.qpp_parse_ir import parse_file  # Local import to avoid heavy deps

        text = Path(source_path).read_text()
        ir = parse_file(text)
        results: List[Any] = []
        qasms = ir.get("qasm", [])
        if qasms:
            for qasm in qasms:
                results.append(self.run_qasm(qasm, shots=shots))
            return results

        ops = ir.get("operations", [])
        if not ops:
            return results

        max_q = 0
        max_c = 0
        for op in ops:
            args = op.get("args", [])
            if not args:
                continue
            if op["gate"].lower() == "measure" and len(args) > 1:
                max_q = max(max_q, args[0])
                max_c = max(max_c, args[1])
            else:
                max_q = max(max_q, max(args))

        tgt = self.target.lower()
        backend = self.connect()
        if tgt == "qiskit":
            if qiskit is None:  # pragma: no cover - runtime check
                raise ImportError("qiskit is not installed")
            qc = qiskit.QuantumCircuit(max_q + 1, max_c + 1)
            for op in ops:
                gate = op["gate"].lower()
                a = op.get("args", [])
                if gate == "h":
                    qc.h(a[0])
                elif gate == "cx":
                    qc.cx(a[0], a[1])
                elif gate == "measure":
                    qc.measure(a[0], a[1])
            job = qiskit.execute(qc, backend=backend, shots=shots)
            results.append(job.result())
        elif tgt == "cirq":
            if cirq is None:  # pragma: no cover - runtime check
                raise ImportError("cirq is not installed")
            qubits = [cirq.LineQubit(i) for i in range(max_q + 1)]
            circuit = cirq.Circuit()
            for op in ops:
                gate = op["gate"].lower()
                a = op.get("args", [])
                if gate == "h":
                    circuit.append(cirq.H(qubits[a[0]]))
                elif gate == "cx":
                    circuit.append(cirq.CNOT(qubits[a[0]], qubits[a[1]]))
                elif gate == "measure":
                    circuit.append(cirq.measure(qubits[a[0]], key=f"c{a[1]}"))
            results.append(backend.run(circuit, repetitions=shots))
        else:
            raise ValueError(f"Unknown target '{self.target}'")

        return results
