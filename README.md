# Q++ GCC

This repository is a fork of GCC with experimental extensions for quantum programming.
For background information on GCC itself see [README](README). The additions here focus on
new language constructs and tooling to support quantum-aware C++ development. Full project
documentation is available on the [Q++ website](https://sefunmi4.github.io/aoh-guild-house/projects/qpp/).

## Q++ Highlights

- **`qstruct` and `qclass`** &mdash; Data structures for quantum state
  storage and manipulation.  See
  [`include/qpp/qstruct.hpp`](include/qpp/qstruct.hpp) for
  reference implementations.
- **Probabilistic `bool`** &mdash; Boolean values that represent
  quantum superposition probabilities. The `qpp/pbool.h` header
  implements a simple `pbool` type that stores the likelihood of
  a value being `true` and provides logical operations that combine
  those probabilities.
- **Bitwise gate macros** &mdash; Convenience wrappers for common
  quantum logic gates such as `H`, `X`, `Y`, `Z`, `CNOT` and `CCX`.
- **Hardware API stubs** &mdash; Placeholder interfaces intended for
  integration with future quantum devices.
- **Hardware detection fallback** &mdash; `qpp::hardware_available()`
  checks for a usable quantum backend and probabilistic types such as
  `qpp::pbool` transparently fall back to a classical random number
  generator with basic glitch detection when none is present.
- **Priority-aware scheduler** &mdash; The `qpp::Scheduler` manages
  `task<*>` functions, supports pause/resume semantics and allows
  removing or reprioritizing tasks.
- **`cstruct` and `cclass`** &mdash; Classical-only variants used for
  hybrid modeling alongside quantum structures.
- **`qregister` and `cregister`** &mdash; Explicit register allocation or
  compiler inference when `register` is left as `auto`.
- **`task<\*>` annotations** &mdash; Functions targeted to the QPU, CPU or
  determined automatically.
- **`__qasm` blocks** &mdash; Inject raw QASM instructions within the
  source code.
- **`#explain` directive** &mdash; Request runtime explanations for upcoming
  instructions.
- **QIR-annotated LLVM IR** &mdash; Compiler output includes Quantum IR with
  probability metadata for hardware backends.
- **`features.yaml` roadmap** &mdash; A YAML file enumerates remaining tasks
  required for a production-ready implementation.

These features are experimental and not part of upstream GCC.

## World selection engine

The `worlds_lib` component implements the "quantum worlds" selection
engine that powers sampling-heavy demos and experimental analyses. It
encodes each candidate world as a prime-based signature, evaluates
overlap in a spectral domain, and then samples from either a classical
or simulated quantum backend.

### Prime-based signatures

`FactorRegistry` assigns unique prime numbers to every symbolic factor
used in a `WorldSignature`. The signature stores weighted labels (for
example `{"cat", 1.0}` or `{"wearing", 1.2}`) that describe the
world. Multiplying the weights by their assigned primes produces a
deterministic spectral fingerprint that is independent of the order in
which the factors were registered.

### Spectral scoring

The spectral representation feeds into a Gaussian overlap kernel that
compares how similar two worlds are. The overlap of a world with itself
and the relationships to its neighbours are combined into a final score:

- `sigma` controls the width of the Gaussian used for the overlap
  computation. Smaller values make the engine more sensitive to small
  spectral deviations.
- `lambda` controls how much weight to place on relationships to other
  worlds versus only the self overlap.
- `temperature` adjusts how peaky or flat the post-processed score
  distribution becomes before sampling.
- `k` is the number of Monte-Carlo samples ("shots") to draw on the CPU
  backend. When set to zero the engine uses the analytical softmax
  distribution instead of sampling.

All of these parameters can be configured from the `world_picker` CLI
flags described below.

### Backend options and simulator path

Two backends are provided:

- `cpu` draws samples using a deterministic RNG seeded per run. This is
  useful for debugging and testing because results are reproducible.
- `qpu_sim` produces normalized amplitudes suitable for a quantum
  simulator and can be squared to obtain probabilities.

If you replace the built-in simulator with a vendor-specific
implementation, set the `QPP_QPU_SIM_PATH` environment variable (or add
the simulator directory to `LD_LIBRARY_PATH`) so that the runtime can
locate the simulator shared library. Without a valid simulator path the
`qpu_sim` backend is unavailable.

## Installing a release

Prebuilt binaries are published on the project's releases page. Download the
archive matching your platform, extract it, and place the `qpp` executable on
your `PATH`.

### Homebrew tap (macOS & Linux)

If you previously attempted to install and `brew list qpp` reported `No such keg`,
refresh the tap and reinstall the formula:

```sh
brew untap sefunmi4/qpp 2>/dev/null
brew tap sefunmi4/qpp
brew install qpp
# or
brew reinstall sefunmi4/qpp/qpp
```

After a successful install, `brew list qpp` should display the installed files.

### Linux

```sh
curl -L -o qpp.tar.gz https://github.com/<owner>/qpp-gcc/releases/download/<tag>/qpp-<tag>-linux-x86_64.tar.gz
tar -xzf qpp.tar.gz
sudo mv qpp-<tag>/bin/qpp /usr/local/bin/
qpp --help
```

### Debian/Ubuntu `.deb` packages

You can also install from a `.deb` release. Ensure the package file is in
your current directory before running:

```sh
sudo apt install ./qpp-0.1.0-ubuntu-x64.deb
# or:
sudo dpkg -i qpp-0.1.0-ubuntu-x64.deb
sudo apt -f install   # fix deps if needed
```

If the `.deb` file is missing, `apt` reports `E: Unsupported file
./qpp-0.1.0-ubuntu-x64.deb given on commandline`, `dpkg` reports
`dpkg: error: cannot access archive 'qpp-0.1.0-ubuntu-x64.deb': No such
file or directory`, and `sudo apt -f install` finds no dependencies to
fix.

### macOS

```sh
curl -L -o qpp-macos.tar.gz https://github.com/<owner>/qpp-gcc/releases/download/<tag>/qpp-<tag>-macos-x86_64.tar.gz
tar -xzf qpp-macos.tar.gz
sudo mv qpp-<tag>/bin/qpp /usr/local/bin/
xattr -dr com.apple.quarantine /usr/local/bin/qpp  # remove quarantine
codesign --force --sign - /usr/local/bin/qpp       # optional ad-hoc signing
qpp --help
```

## Uninstalling

The `qpp` binary can be removed by reversing the installation steps.

### Homebrew

```sh
brew uninstall qpp
brew untap sefunmi4/qpp  # optional
```

### Manual download

Delete the `qpp` executable you placed on your `PATH`:

```sh
sudo rm /usr/local/bin/qpp
```

### Source build

`make uninstall` is not supported. Remove the installation prefix
you supplied to `configure`:

```sh
sudo rm -rf /usr/local/gcc-x86_64
```

Run `which qpp` to confirm the command is no longer available.

## Building from source

The Q++ fork retains GCC's traditional `configure`/`make` build
system. The process below mirrors the steps outlined in
[CONTRIBUTING](.github/CONTRIBUTING.md).

```sh
# Download prerequisite libraries (GMP, MPFR, MPC, etc.)
./contrib/download_prerequisites

# Ensure required build tools are installed
# gawk and bison >= 3.0 must be available in your PATH
# (e.g., on macOS: brew install gawk bison)

# Create a separate build directory
mkdir build && cd build

# Configure the build (adjust --enable-languages as needed)
../configure --disable-multilib --enable-languages=c,c++

# Compile
make -j"$(nproc)"

# Run the test suite
make -k check

# Optionally install
make install
```

### Building the world-selection tooling

The CMake build also produces the standalone world-selection
artifacts. From the repository root:

```sh
cmake -S . -B build -G Ninja   # or your preferred generator
cmake --build build --target worlds_lib
cmake --build build --target world_picker
```

The `worlds_lib` target contains the reusable selection engine and is a
dependency of both the CLI and the tests. Substitute `--target
worlds_lib worlds_tests` if you want to compile the library and the test
binary in one command.

### Running the World Picker CLI

Invoke the CLI directly from the build directory. Parameters default to
the values shown in the example below and can be overridden with
`--backend`, `--k`, `--sigma`, `--lambda`, and `--temperature`:

```sh
./build/world_picker --backend cpu --k 128 --sigma 1.0 --lambda 0.5 --temperature 1.0
```

Sample output (with the default deterministic seed) looks like:

```text
Quantum World Picker
Backend: CPU
k (shots): 128
sigma: 1
lambda: 0.5
temperature: 1

World             Score        CPU Prob        QPU Prob  Signature
--------------------------------------------------------------------------------
cat                 4.9645          0.5938          0.5768  cat:1,feline:0.8,pet:0.6
hat                 2.0303          0.0469          0.0307  accessory:0.9,fashion:0.7,hat:1
cat_hat_pair        2.7537          0.0703          0.0632  cat:0.9,ensemble:1.1,hat:0.9
cat_wearing_hat     3.3260          0.0625          0.1121  cat:1,hat:1,relationship:0.8,wearing:1.2
hat_on_cat          3.4915          0.1562          0.1322  balance:0.6,cat:0.95,hat:1,on_top:1.3
cat_hat_story       3.0501          0.0703          0.0850  adventure:1.1,cat:0.85,hat:0.85,narrative:0.75

Sampled world (deterministic RNG): cat
Most probable for CPU: cat (0.5938)
```

Switch `--backend qpu_sim` to view the simulated quantum amplitudes.
When pointing the CLI at an external simulator, ensure the
`QPP_QPU_SIM_PATH` variable is set as described in the world-selection
engine overview.

### Running the world-selection tests

Compile the test binary and execute it either directly or via CTest:

```sh
cmake --build build --target worlds_tests
./build/worlds_tests
# or
ctest --test-dir build --output-on-failure -R worlds_tests
```

### macOS build

If the generic steps above fail on macOS, the sequence below has been tested
to work on recent systems. It targets an x86_64 toolchain using the macOS SDK
under Rosetta and avoids the missing `/usr/include` and `/lib/cpp` issues on
modern installations while keeping the build in stage1.

```sh
# Rosetta shell: obtain the SDK path
SDKROOT="$(arch -x86_64 xcrun --sdk macosx --show-sdk-path)"
echo "$SDKROOT"

# Start from a clean build directory
rm -rf build-gcc
mkdir build-gcc && cd build-gcc

# Configure the environment to use the SDK and clang's preprocessor
export CC="clang -arch x86_64"
export CXX="clang++ -arch x86_64"
export CPP="/usr/bin/clang -E -arch x86_64"

export CPPFLAGS="-I/usr/local/include -I/usr/local/opt/zlib/include -isysroot $SDKROOT"
export CFLAGS="-isysroot $SDKROOT -mmacosx-version-min=11.0"
export CXXFLAGS="$CFLAGS"
export LDFLAGS="-L/usr/local/lib -L/usr/local/opt/zlib/lib -Wl,-syslibroot,$SDKROOT -mmacosx-version-min=11.0"

# Configure with the SDK and a reduced feature set
../configure \
  --build=x86_64-apple-darwin23 \
  --host=x86_64-apple-darwin23 \
  --target=x86_64-apple-darwin23 \
  --prefix=/usr/local/gcc-x86_64 \
  --with-sysroot="$SDKROOT" \
  --with-gmp=/usr/local/opt/gmp \
  --with-mpfr=/usr/local/opt/mpfr \
  --with-mpc=/usr/local/opt/libmpc \
  --with-isl=/usr/local/opt/isl \
  --with-system-zlib \
  --enable-languages=c,c++ \
  --disable-multilib \
  --disable-nls \
  --disable-libgomp \
  --disable-bootstrap

# Build and install
make -j"$(sysctl -n hw.ncpu)"
sudo make install

# Verify the installation
export PATH="/usr/local/gcc-x86_64/bin:$PATH"
file /usr/local/gcc-x86_64/bin/gcc
/usr/local/gcc-x86_64/bin/gcc -v
/usr/local/gcc-x86_64/bin/g++ -v

cat > helloworld.qpp <<'QPP'
#include <iostream>
int main(){ std::cout << "hello from q++!\n"; }
QPP
/usr/local/gcc-x86_64/bin/g++ -std=c++17 -x c++ helloworld.qpp -o helloworld
./helloworld
```

### Example

After building, you can run the boolean expression demo directly from its
`.qpp` source using the `qpp` command-line tool:

```sh
# from the repository root after building with CMake
./build/qpp examples/pbool_demo.qpp

# or with an installed release
qpp examples/pbool_demo.qpp
```
Alternatively, the `g++` commands below compile and run the corresponding C++
demo sources (`pbool_demo.cpp`, etc.) so you can compare the `.qpp` and C++
versions:

```sh
/usr/local/gcc-x86_64/bin/g++ -std=c++17 -Iinclude examples/pbool_demo.cpp -o pbool_demo
./pbool_demo
```

To explore the circuit API, the `examples/bell_state.cpp` program
creates a Bell state using `Circuit` and `Program` with the
`LocalSimBackend` and prints the resulting measurement
probabilities:

```sh
/usr/local/gcc-x86_64/bin/g++ -std=c++17 -Iinclude examples/bell_state.cpp \
    qpp/backend/LocalSimBackend.cpp -o bell_state
./bell_state
```

To compare hardware and simulation backends, the
`examples/hardware_demo.cpp` program selects a hardware device when one is
available and falls back to `LocalSimBackend` otherwise. It reports timing
and throughput for both backends:

```sh
/usr/local/gcc-x86_64/bin/g++ -std=c++17 -Iinclude examples/hardware_demo.cpp \
    qpp/backend/LocalSimBackend.cpp qpp/backend/QPUBackend.cpp -o hardware_demo
# Optional: simulate a hardware device
QPU_PCI_DEVICE=dummy ./hardware_demo
# Without a device it falls back to LocalSimBackend
./hardware_demo
```

Alternatively, the examples can be built using CMake:

```sh
cmake -S examples -B build/examples
cmake --build build/examples --target bell_state
./build/examples/bell_state
```

### Parsing Example

The repository includes a small prototype parser that emits a JSON
representation of Q++ constructs. Run it on the provided sample file:

```sh
python3 contrib/qpp_parse_ir.py examples/qpp_parse_example.qpp
```

Additional design notes can be found in the files under
`docs/architecture`, covering the frontend, runtime and hardware API.

Refer to the upstream GCC README for additional information about the
compiler and licensing.

