# tensorium-clang-audit

Minimal C++17 Clang frontend plugin for Tensorium audit checks.

This project does not vendor LLVM or Clang. It expects an existing local LLVM/Clang installation and builds a shared plugin library named `TensoriumClangAudit`.

The plugin currently reports:

- C++ `new` / `new[]`
- C++ `delete` / `delete[]`
- C `malloc`, `calloc`, `realloc`, and `free`
- expensive math calls inside loops: `std::pow`, `std::sqrt`, `std::sin`, `std::cos`

Diagnostics use stable `TCAxxx` identifiers:

- `TCA001`: C++ allocation expression inside loop
- `TCA002`: C++ deallocation expression inside loop
- `TCA003`: C allocation call inside loop
- `TCA004`: C deallocation call inside loop
- `TCA005`: expensive math function call inside loop
- `TCA006`: allocation or deallocation inside nested loop

Each warning includes a follow-up note with a short remediation hint.

## Build

```sh
mkdir -p build
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DLLVM_DIR=/opt/local/libexec/llvm-20/lib/cmake/llvm \
  -DClang_DIR=/opt/local/libexec/llvm-20/lib/cmake/clang

cmake --build build -j
```

Or use the helper script:

```sh
./scripts/build.sh
```

`scripts/build.sh` uses `LLVM_DIR` and `Clang_DIR` from the environment if they are set. Otherwise it defaults to the MacPorts LLVM 20 paths under `/opt/local/libexec/llvm-20`.

## Run

```sh
clang++ -std=c++17 \
  -Xclang -load -Xclang ./build/libTensoriumClangAudit.so \
  -Xclang -plugin -Xclang tensorium-clang-audit \
  -c examples/sample.cpp
```

Or use:

```sh
./scripts/run_sample.sh
```

Plugin options can be passed with Clang's frontend plugin argument syntax:

```sh
clang++ -std=c++17 \
  -Xclang -load -Xclang ./build/libTensoriumClangAudit.so \
  -Xclang -plugin -Xclang tensorium-clang-audit \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang -checks=alloc-in-loop \
  -c examples/sample.cpp
```

Supported options:

- `-quiet`: accept quiet mode for non-diagnostic plugin output
- `-checks=alloc-in-loop`: enable the allocation-in-loop check
- `-checks=math-in-loop`: enable the math-in-loop check
- `-checks=all`: enable all checks
- `-checks=none`: disable all checks

## Test

```sh
./scripts/test.sh
```

The test script builds the plugin and runs positive and negative fixtures from `tests/fixtures`.
