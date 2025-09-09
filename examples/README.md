# QPP Examples

These samples demonstrate how to use the qpp APIs and backends. Source files now
use the `.qpp` extension, though the build system still accepts `.cpp` sources
for backward compatibility.

## Build

```
cd examples
mkdir build && cd build
cmake ..
make
```

## Run

After building, the example binaries reside in `examples/build`.

To execute the hardware demo and compare a discovered device with the
local simulator:

```
# Optional: simulate a hardware device
QPU_PCI_DEVICE=dummy ./hardware_demo

# Without a device it falls back to LocalSimBackend
./hardware_demo
```

The program reports timing and throughput for the selected backend and for
`LocalSimBackend` for comparison.

## master_demo.cpp

Compile and run directly:

```
g++ -std=c++17 -I include -I . examples/master_demo.cpp && ./a.out
```
