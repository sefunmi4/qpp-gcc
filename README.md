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

## Building from source

The Q++ fork retains GCC's traditional `configure`/`make` build
system. The process below mirrors the steps outlined in
[CONTRIBUTING](.github/CONTRIBUTING.md).

```sh
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

The following commands build the toolchain for an x86_64 target using the
macOS SDK and clang as the host compiler. These steps avoid the missing
`/usr/include` and `/lib/cpp` issues on modern macOS and keep the build in
stage1.

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

cat > hello.cpp <<'CPP'
#include <iostream>
int main(){ std::cout << "hello from g++!\n"; }
CPP
/usr/local/gcc-x86_64/bin/g++ -std=c++17 hello.cpp -o hello
./hello
```

Prebuilt macOS binaries are periodically published on the project's
releases page. Check there if you prefer not to build from source.

### Example

After building, you can compile the demonstration program in
`examples/pbool_demo.cpp` using the freshly built compiler:

```sh
./build/gcc/g++ -Iinclude examples/pbool_demo.cpp -o pbool_demo
./pbool_demo
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

