#!/usr/bin/env bash
set -euo pipefail

LLVM_DIR="${LLVM_DIR:-/opt/local/libexec/llvm-20/lib/cmake/llvm}"
Clang_DIR="${Clang_DIR:-/opt/local/libexec/llvm-20/lib/cmake/clang}"

cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DLLVM_DIR="${LLVM_DIR}" \
  -DClang_DIR="${Clang_DIR}"

cmake --build build -j
