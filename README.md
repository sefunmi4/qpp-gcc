# Q++ GCC

This repository is a fork of GCC with experimental extensions for quantum programming.
For background information on GCC itself see [README](README). The additions here focus on
new language constructs and tooling to support quantum-aware C++ development.

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

