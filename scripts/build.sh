#!/usr/bin/env bash
set -euo pipefail

LLVM_DIR="${LLVM_DIR:-/opt/local/libexec/llvm-20/lib/cmake/llvm}"
Clang_DIR="${Clang_DIR:-/opt/local/libexec/llvm-20/lib/cmake/clang}"

CMAKE_ARGS=(
  -S .
  -B build
  -DCMAKE_BUILD_TYPE=Debug \
  -DLLVM_DIR="${LLVM_DIR}" \
  -DClang_DIR="${Clang_DIR}"
)

if [[ -f build/CMakeCache.txt ]] &&
  grep -q "${PWD}/.bear/" build/CMakeCache.txt &&
  [[ ! -d .bear ]]; then
  CMAKE_ARGS+=(
    -DCMAKE_C_COMPILER="${CC:-/usr/bin/cc}"
    -DCMAKE_CXX_COMPILER="${CXX:-/usr/bin/c++}"
  )
fi

cmake "${CMAKE_ARGS[@]}"
cmake --build build -j
