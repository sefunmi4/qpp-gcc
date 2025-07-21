# TODO

## Short-Term Tasks

- Implement parser/IR generation for quantum instructions
- Develop runtime scheduler
- Add extra gates to the instruction set
- Ensure thread safety across components
- Expand test coverage

## Longer-Term Goals

- Support conditional gates
- Integrate with hardware backends
- Improve scheduler controls
- Compiler front-end parses simple control flow and emits an intermediate
  representation used by `qpp-run`.
- `qpp-run` executes that IR via the scheduler and wavefunction backend,
  supporting conditional gates.

