# Hardware API

This file sketches the interface expected by future Q++ hardware
backends. A hardware profile is described in a simple YAML format and
defines qubit count, supported gates and rate limits.

The runtime emits QIR strings when executing tasks. Real backends would
translate this intermediate form to device specific commands. For now
`HardwareStub` simply stores the strings for inspection.

The API is intentionally minimal:

```cpp
class HardwareStub {
  void emit(const std::string& qir);
  std::string result() const;
};
```

Implementations may choose to stream operations, perform buffering or
apply other optimizations. Rate-limit handling and multi-backend support
are left for future work.

