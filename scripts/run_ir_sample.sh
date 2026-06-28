#!/usr/bin/env bash
set -euo pipefail

PLUGIN_PATH="./build/libTensoriumLLVMIRAudit.so"
DEFAULT_LLVM_BIN="/opt/local/libexec/llvm-20/bin"
TMP_DIR="${TMPDIR:-/tmp}/tensorium-clang-audit-ir"
IR_FILE="${TMP_DIR}/sample.ll"

if [[ -x "${DEFAULT_LLVM_BIN}/clang++" ]]; then
  CLANGXX="${CLANGXX:-${DEFAULT_LLVM_BIN}/clang++}"
else
  CLANGXX="${CLANGXX:-clang++}"
fi

if [[ -x "${DEFAULT_LLVM_BIN}/opt" ]]; then
  OPT="${OPT:-${DEFAULT_LLVM_BIN}/opt}"
else
  OPT="${OPT:-opt}"
fi

mkdir -p "${TMP_DIR}"

"${CLANGXX}" -std=c++17 -S -emit-llvm examples/sample.cpp -o "${IR_FILE}"

"${OPT}" \
  -load-pass-plugin "${PLUGIN_PATH}" \
  -passes=tensorium-ir-audit \
  -disable-output \
  "${IR_FILE}"
