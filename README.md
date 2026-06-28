# tensorium-clang-audit

Minimal C++17 Clang frontend plugin for Tensorium audit checks.

This project does not vendor LLVM or Clang. It expects an existing local LLVM/Clang installation and builds a shared plugin library named `TensoriumClangAudit`.

## Layout

The repository is split so the Clang frontend plugin can grow without blocking a future LLVM IR plugin/pass:

```txt
include/TensoriumClangAudit/
  Clang/
    Checks/      # Clang AST check classifiers
    Support/     # Clang diagnostic and source helpers
    Options.hpp
    Plugin.hpp
    Visitor.hpp
  LLVM/
    Passes/      # LLVM IR pass skeletons
    Plugin.hpp
src/
  clang/
    Checks/
    Support/
    Plugin.cpp
    Visitor.cpp
  llvm/
    Passes/
    Plugin.cpp
tests/
  fixtures/
```

Top-level headers such as `include/TensoriumClangAudit/Visitor.hpp` are compatibility wrappers around the `Clang/` headers.
The LLVM IR side builds as a separate plugin target named `TensoriumLLVMIRAudit`.

The plugin currently reports:

- C++ `new` / `new[]`
- C++ `delete` / `delete[]`
- C `malloc`, `calloc`, `realloc`, and `free`
- expensive math calls inside loops: `std::pow`, `std::sqrt`, `std::sin`, `std::cos`
- STL container growth inside loops: `push_back`, `emplace_back`
- C and C++ I/O inside loops: `printf`, `fprintf`, `std::cout`, `std::cerr`
- virtual calls inside loops

Diagnostics use stable `TCAxxx` identifiers:

- `TCA001`: C++ allocation expression inside loop
- `TCA002`: C++ deallocation expression inside loop
- `TCA003`: C allocation call inside loop
- `TCA004`: C deallocation call inside loop
- `TCA005`: expensive math function call inside loop
- `TCA006`: allocation or deallocation inside nested loop
- `TCA007`: loop-invariant expensive math function call inside loop
- `TCA008`: STL container growth inside loop
- `TCA009`: I/O operation inside loop
- `TCA010`: virtual call inside loop

Each warning includes a follow-up note with a short remediation hint.

Suppress a diagnostic on the next line with:

```cpp
// tensorium-clang-audit: disable-next-line TCA008
values.push_back(i);
```

Use `all` to suppress every Tensorium diagnostic on the next line:

```cpp
// tensorium-clang-audit: disable-next-line all
std::printf("%d\n", i);
```

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

### Clang frontend plugin

Basic plugin invocation:

```sh
clang++ -std=c++17 \
  -Xclang -load -Xclang ./build/libTensoriumClangAudit.so \
  -Xclang -plugin -Xclang tensorium-clang-audit \
  -c examples/sample.cpp
```

To select loop analyses, add `-plugin-arg-tensorium-clang-audit` followed by
the option:

```sh
clang++ -std=c++17 \
  -Xclang -load -Xclang ./build/libTensoriumClangAudit.so \
  -Xclang -plugin -Xclang tensorium-clang-audit \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang --loop-analyse=alloc \
  -c examples/sample.cpp
```

Enable all loop analyses:

```sh
clang++ -std=c++17 \
  -Xclang -load -Xclang ./build/libTensoriumClangAudit.so \
  -Xclang -plugin -Xclang tensorium-clang-audit \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang --loop-analyse=all \
  -c examples/sample.cpp
```

Or use:

```sh
./scripts/run_sample.sh
```

`run_sample.sh` forwards extra arguments to Clang, so this is equivalent:

```sh
./scripts/run_sample.sh \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang --loop-analyse=maths
```

Supported options:

- `-quiet`: accept quiet mode for non-diagnostic plugin output
- `--loop-analyse=all`: enable every loop analysis
- `--loop-analyse=none`: disable loop analysis
- `--loop-analyse=alloc`: enable allocation/deallocation loop analysis
- `--loop-analyse=maths`: enable expensive math loop analysis
- `--loop-analyse=stl`: enable STL growth loop analysis
- `--loop-analyse=io`: enable I/O loop analysis
- `--loop-analyse=virtual-calls`: enable virtual-call loop analysis
- `--loop-analyse=alloc,maths,stl,io,virtual-calls`: enable selected loop analyses

Legacy `-checks=all`, `-checks=none`, `-checks=alloc-in-loop`, and `-checks=math-in-loop` are still accepted as compatibility aliases.

### LLVM IR plugin

The LLVM IR skeleton pass is loadable with `opt`:

```sh
clang++ -std=c++17 -S -emit-llvm examples/sample.cpp -o /tmp/tensorium-sample.ll

opt \
  -load-pass-plugin ./build/libTensoriumLLVMIRAudit.so \
  -passes=tensorium-ir-audit \
  -disable-output \
  /tmp/tensorium-sample.ll
```

Or use:

```sh
./scripts/run_ir_sample.sh
```

The IR pass currently only prints the module and function names it visits.
Real LLVM IR analysis will be added later.

## Test

```sh
./scripts/test.sh
```

The test script builds the plugin and runs positive and negative fixtures from `tests/fixtures`.
Filtered diagnostics are compared against golden files in `tests/expected`.
