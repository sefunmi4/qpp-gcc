# Scheduler Demo

## Overview
`scheduler_demo.qpp` showcases the cooperative task scheduler that orchestrates quantum and classical work. It adds three lambda tasks, adjusts priorities, removes one task, and streams the resulting QIR instructions via a `HardwareStub` while mirroring the updates in a simulated `qregister`.

## Highlights
- `qpp::Scheduler` holds queued lambdas, each manipulating both the stub backend and an in-memory quantum register.
- Priorities can be reassigned after submission; the example boosts the second task before execution.
- Tasks are removable by index, allowing dynamic pruning before `run()` executes the queue.

## Build and Run
### CMake
```sh
cd examples
mkdir -p build && cd build
cmake ..
make scheduler_demo
./scheduler_demo
```
The output displays the QIR emitted by the two remaining tasks in priority order.

### Stand-alone Compilation
The implementation lives entirely in headers, so compiling the `.cpp` wrapper is sufficient from the repository root:
```sh
g++ -std=c++17 -Iinclude -I. examples/scheduler_demo.cpp -o scheduler_demo
./scheduler_demo
```
