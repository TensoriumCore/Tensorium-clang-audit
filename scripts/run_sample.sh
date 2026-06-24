#!/usr/bin/env bash
set -euo pipefail

PLUGIN_PATH="./build/libTensoriumClangAudit.so"
DEFAULT_CLANG="/opt/local/libexec/llvm-20/bin/clang++"

if [[ -x "${DEFAULT_CLANG}" ]]; then
  CLANGXX="${CLANGXX:-${DEFAULT_CLANG}}"
else
  CLANGXX="${CLANGXX:-clang++}"
fi

"${CLANGXX}" -std=c++17 \
  -Xclang -load -Xclang "${PLUGIN_PATH}" \
  -Xclang -plugin -Xclang tensorium-clang-audit \
  -c examples/sample.cpp
