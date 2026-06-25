# tensorium-clang-audit

Minimal C++17 Clang frontend plugin for Tensorium audit checks.

This project does not vendor LLVM or Clang. It expects an existing local LLVM/Clang installation and builds a shared plugin library named `TensoriumClangAudit`.

The plugin currently reports dynamic allocation and deallocation inside loops:

- C++ `new` / `new[]`
- C++ `delete` / `delete[]`
- C `malloc`, `calloc`, `realloc`, and `free`

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

## Test

```sh
./scripts/test_sample.sh
```

The test script builds the plugin, runs it on `examples/sample.cpp`, and checks that the expected warnings are emitted.
