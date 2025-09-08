# QPP Examples

These samples demonstrate how to use the qpp APIs and backends.

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
