# qpp API

## Backend selection

`Program` instances obtain a backend via `BackendFactory`.  By default the
factory prefers a hardware `QPUBackend` when available and falls back to the
`LocalSimBackend` simulator.

The selection can be overridden:

* Set the environment variable `QPP_BACKEND` to either `qpu` or `sim`.
* Create a `qpp_backend.conf` file with `qpu` or `sim` on its first line.

If a selection is forced, the factory returns the requested backend without
probing.  Otherwise it attempts to connect to hardware and uses the simulator
when no device is found.
