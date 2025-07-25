# Runtime Overview

This document describes the prototype runtime environment used by Q++.
It complements the frontend overview and focuses on task execution and
hardware integration.

## Scheduler

The `qpp::Scheduler` class manages a list of tasks. Each task carries a
priority and may be paused or resumed. Tasks can also be removed,
cleared, or reprioritized at runtime. When `run()` is invoked the tasks
are executed in priority order. This simple model is intended to mimic a
more complete quantum/classical scheduler that would handle device
selection and probabilistic control flow.

All public methods are protected by a mutex so that tasks may be added,
paused or resumed from multiple threads. This ensures thread safety when
using `task<CPU>`, `task<QPU>`, `task<AUTO>` or `task<MIXED>` functions
across different components.

## Registers

`qregister` and `cregister` provide explicit quantum and classical
storage. A `qregister` wraps a `qclass` instance and exposes import and
export helpers so that state can be saved or restored. `cregister` stores
ordinary integer bits for hybrid algorithms.

## Raw Gate Injection

Code may contain `__qasm { ... }` blocks. The parser collects the
contents of these blocks and the runtime forwards them unchanged to the
selected backend.

## Device Stubs

`HardwareStub` is a minimal backend that simply collects emitted QIR
strings. It serves as a placeholder until real devices are connected.

## Probability Metadata

Measurements and conditional branches that depend on quantum state are
tagged with probability information.  This metadata is preserved through
the LLVM pass pipeline and attached to the generated QIR so that
hardware backends can reason about likely outcomes.

## Inline Explanations

When a `#explain` directive is encountered the runtime emits a textual
annotation describing upcoming operations. These explanations may be
consumed by tooling or displayed to the user during simulation.

