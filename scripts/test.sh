#!/usr/bin/env bash
set -euo pipefail

if ! BUILD_OUTPUT="$("./scripts/build.sh" 2>&1)"; then
  printf '[build]\nfailed\n%s\n' "${BUILD_OUTPUT}" >&2
  exit 1
fi
printf '[build]\nok\n'

PLUGIN_PATH="./build/libTensoriumClangAudit.so"
DEFAULT_CLANG="/opt/local/libexec/llvm-20/bin/clang++"
TMP_DIR="${TMPDIR:-/tmp}/tensorium-clang-audit-tests"

if [[ -x "${DEFAULT_CLANG}" ]]; then
  CLANGXX="${CLANGXX:-${DEFAULT_CLANG}}"
else
  CLANGXX="${CLANGXX:-clang++}"
fi

mkdir -p "${TMP_DIR}"

run_plugin() {
  local source_file="$1"
  shift

  "${CLANGXX}" -std=c++17 \
    -Xclang -load -Xclang "${PLUGIN_PATH}" \
    -Xclang -plugin -Xclang tensorium-clang-audit \
    "$@" \
    -c "${source_file}" \
    -o "${TMP_DIR}/test.o" 2>&1
}

filter_tensorium_output() {
  local output="$1"

  sed -n -E \
    -e 's/^.*warning: (TCA[0-9]+: .*)$/\1/p' \
    -e 's/^.*note: (consider .*)$/note: \1/p' \
    <<<"${output}"
}

run_filtered() {
  local raw_output

  raw_output="$(run_plugin "$@")"
  filter_tensorium_output "${raw_output}"
}

run_case() {
  local label="$1"
  local expected_file="$2"
  local source_file="$3"
  shift 3

  local actual_file="${TMP_DIR}/${label}.actual"
  run_filtered "${source_file}" "$@" >"${actual_file}"

  printf '\n[%s]\n' "${label}"
  if [[ -s "${actual_file}" ]]; then
    cat "${actual_file}"
  else
    printf 'no Tensorium diagnostics\n'
  fi

  if ! diff -u "${expected_file}" "${actual_file}"; then
    printf 'golden mismatch for %s\n' "${label}" >&2
    exit 1
  fi
}

run_case "loop-analyse=alloc" tests/expected/alloc_in_loop.txt \
  tests/fixtures/alloc_in_loop.cpp \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang --loop-analyse=alloc

run_case "loop-analyse=maths" tests/expected/math_in_loop.txt \
  tests/fixtures/math_in_loop.cpp \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang --loop-analyse=maths

run_case "loop-analyse=all" tests/expected/math_in_loop.txt \
  tests/fixtures/math_in_loop.cpp \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang --loop-analyse=all

run_case "loop-analyse=stl" tests/expected/stl_in_loop.txt \
  tests/fixtures/stl_in_loop.cpp \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang --loop-analyse=stl

run_case "loop-analyse=io" tests/expected/io_in_loop.txt \
  tests/fixtures/io_in_loop.cpp \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang --loop-analyse=io

run_case "loop-analyse=virtual-calls" tests/expected/virtual_call_in_loop.txt \
  tests/fixtures/virtual_call_in_loop.cpp \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang --loop-analyse=virtual-calls

run_case "legacy checks=alloc-in-loop" tests/expected/alloc_in_loop.txt \
  tests/fixtures/alloc_in_loop.cpp \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang -checks=alloc-in-loop

run_case "loop-analyse=none" tests/expected/empty.txt \
  tests/fixtures/alloc_in_loop.cpp \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang --loop-analyse=none

run_case "quiet loop-analyse=alloc" tests/expected/alloc_in_loop.txt \
  tests/fixtures/alloc_in_loop.cpp \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang -quiet \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang --loop-analyse=alloc

run_case "negative alloc fixture" tests/expected/empty.txt \
  tests/fixtures/no_alloc_in_loop.cpp

run_case "negative math fixture" tests/expected/empty.txt \
  tests/fixtures/no_math_in_loop.cpp \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang --loop-analyse=maths

run_case "negative stl fixture" tests/expected/empty.txt \
  tests/fixtures/no_stl_in_loop.cpp \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang --loop-analyse=stl

run_case "negative io fixture" tests/expected/empty.txt \
  tests/fixtures/no_io_in_loop.cpp \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang --loop-analyse=io

run_case "negative virtual-call fixture" tests/expected/empty.txt \
  tests/fixtures/no_virtual_call_in_loop.cpp \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang --loop-analyse=virtual-calls

printf '\nall plugin tests passed\n'
