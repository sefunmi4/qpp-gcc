#!/usr/bin/env python3
#
# Simple parser/IR generator for the experimental Q++ extensions.
# This is not a full C++ parser. It only understands a handful of
# quantum-aware constructs and emits a JSON representation.  The goal
# is to provide a starting point for more complete frontend work.
#
# This file is part of GCC and released under the GPLv3 or later.
#
# Usage: qpp_parse_ir.py source.qpp -o out.json

import argparse
import json
import re
from pathlib import Path


def parse_file(text: str) -> dict:
    """Return a very small IR extracted from the given source."""
    result = {
        "structs": [],
        "vars": [],
        "tasks": [],
        "operations": [],
        "qasm": []
    }

    lines = text.splitlines()
    qasm_mode = False
    qasm_buffer = []

    for line in lines:
        stripped = line.strip()

        if qasm_mode:
            if '}' in stripped:
                qasm_buffer.append(stripped.split('}', 1)[0].strip())
                result["qasm"].append("\n".join(qasm_buffer))
                qasm_buffer = []
                qasm_mode = False
            else:
                qasm_buffer.append(stripped)
            continue

        if '__qasm' in stripped:
            # start raw QASM capture
            if '{' in stripped:
                after = stripped.split('{', 1)[1]
                if '}' in after:
                    result["qasm"].append(after.split('}', 1)[0].strip())
                else:
                    qasm_mode = True
                    qasm_buffer.append(after.strip())
            continue

        m = re.search(r'\b(qstruct|qclass|cstruct|cclass)\s+(\w+)', stripped)
        if m:
            kind, name = m.groups()
            result["structs"].append({"kind": kind, "name": name})
            continue

        m = re.search(r'\b(qregister|cregister|register)\s+(\w+)', stripped)
        if m:
            kind, name = m.groups()
            result["vars"].append({"kind": kind, "name": name})
            continue

        m = re.search(r'\btask<([A-Z]+)>\s+(\w+)\s*\(', stripped)
        if m:
            target, name = m.groups()
            result["tasks"].append({"name": name, "target": target})
            continue

        if re.search(r'bool\s+\w+\s*=.*q\[', stripped):
            result["operations"].append({"type": "probabilistic_bool", "text": stripped})
            continue

        if '^' in stripped:
            result["operations"].append({"type": "cx", "text": stripped})
        if '&' in stripped:
            result["operations"].append({"type": "toffoli", "text": stripped})

    return result


def main() -> None:
    ap = argparse.ArgumentParser(description="Q++ demo parser")
    ap.add_argument('source', help='Source file to parse')
    ap.add_argument('-o', '--output', help='Write IR to file')
    args = ap.parse_args()

    text = Path(args.source).read_text()
    ir = parse_file(text)

    if args.output:
        Path(args.output).write_text(json.dumps(ir, indent=2))
    else:
        print(json.dumps(ir, indent=2))


if __name__ == '__main__':
    main()

