# tensorium-clang-audit

Minimal C++17 Clang frontend plugin skeleton for future Tensorium audit checks.

This project does not vendor LLVM or Clang. It expects an existing local LLVM/Clang installation and builds a shared plugin library named `TensoriumClangAudit`.

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

The plugin currently only prints a basic message and the names of visited function declarations. Real audit logic will be added later.
