#!/usr/bin/env bash
set -euo pipefail

if ! BUILD_OUTPUT="$("./scripts/build.sh" 2>&1)"; then
  printf '[build]\nfailed\n%s\n' "${BUILD_OUTPUT}" >&2
  exit 1
fi
printf '[build]\nok\n'

PLUGIN_PATH="./build/libTensoriumClangAudit.so"
DEFAULT_CLANG="/opt/local/libexec/llvm-20/bin/clang++"

if [[ -x "${DEFAULT_CLANG}" ]]; then
  CLANGXX="${CLANGXX:-${DEFAULT_CLANG}}"
else
  CLANGXX="${CLANGXX:-clang++}"
fi

run_plugin() {
  local source_file="$1"
  shift

  "${CLANGXX}" -std=c++17 \
    -Xclang -load -Xclang "${PLUGIN_PATH}" \
    -Xclang -plugin -Xclang tensorium-clang-audit \
    "$@" \
    -c "${source_file}" \
    -o /tmp/tensorium-clang-audit-test.o 2>&1
}

filter_tensorium_output() {
  local output="$1"

  sed -n -E \
    -e 's/^.*warning: (TCA[0-9]+: .*)$/\1/p' \
    -e 's/^.*note: (consider .*)$/note: \1/p' \
    <<<"${output}"
}

expect_output() {
  local output="$1"
  local needle="$2"

  if ! grep -Fq "${needle}" <<<"${output}"; then
    printf 'missing expected filtered output: %s\n' "${needle}" >&2
    printf '%s\n' "${output}" >&2
    exit 1
  fi
}

expect_warning_count() {
  local output="$1"
  local expected="$2"
  local actual

  actual="$(grep -c '^TCA[0-9][0-9][0-9]:' <<<"${output}" || true)"
  if [[ "${actual}" -ne "${expected}" ]]; then
    printf 'expected %s filtered warnings, got %s\n' "${expected}" "${actual}" >&2
    printf '%s\n' "${output}" >&2
    exit 1
  fi
}

expect_no_diagnostics() {
  local output="$1"

  if [[ -n "${output}" ]]; then
    printf 'expected no filtered diagnostics, got:\n%s\n' "${output}" >&2
    exit 1
  fi
}

log_diagnostics() {
  local label="$1"
  local output="$2"

  printf '\n[%s]\n' "${label}"
  if [[ -n "${output}" ]]; then
    printf '%s\n' "${output}"
  else
    printf 'no Tensorium diagnostics\n'
  fi
}

run_filtered() {
  local raw_output

  raw_output="$(run_plugin "$@")"
  filter_tensorium_output "${raw_output}"
}

ALLOC_OUTPUT="$(run_filtered tests/fixtures/alloc_in_loop.cpp \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang -checks=alloc-in-loop)"
log_diagnostics "alloc-in-loop" "${ALLOC_OUTPUT}"

expect_output "${ALLOC_OUTPUT}" "TCA001: C++ allocation expression inside loop"
expect_output "${ALLOC_OUTPUT}" "TCA002: C++ deallocation expression inside loop"
expect_output "${ALLOC_OUTPUT}" "TCA003: C allocation call inside loop"
expect_output "${ALLOC_OUTPUT}" "TCA004: C deallocation call inside loop"
expect_output "${ALLOC_OUTPUT}" "TCA006: allocation or deallocation inside nested loop"
expect_output "${ALLOC_OUTPUT}" "note: consider reusing storage outside the loop"
expect_output "${ALLOC_OUTPUT}" "note: consider moving ownership cleanup outside the loop"
expect_output "${ALLOC_OUTPUT}" "note: consider allocating once before the loop"
expect_output "${ALLOC_OUTPUT}" "note: consider freeing loop-owned storage after the loop"
expect_output "${ALLOC_OUTPUT}" "note: consider moving allocation outside the outer loop"
expect_warning_count "${ALLOC_OUTPUT}" 10

MATH_OUTPUT="$(run_filtered tests/fixtures/math_in_loop.cpp \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang -checks=math-in-loop)"
log_diagnostics "math-in-loop" "${MATH_OUTPUT}"

expect_output "${MATH_OUTPUT}" "TCA005: expensive math function call inside loop"
expect_output "${MATH_OUTPUT}" "note: consider hoisting loop-invariant math"
expect_output "${MATH_OUTPUT}" "TCA007: loop-invariant expensive math function call inside loop"
expect_output "${MATH_OUTPUT}" "note: consider computing this value once before the loop"
expect_warning_count "${MATH_OUTPUT}" 6

DISABLED_OUTPUT="$(run_filtered tests/fixtures/alloc_in_loop.cpp \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang -checks=none)"
log_diagnostics "checks=none" "${DISABLED_OUTPUT}"
expect_no_diagnostics "${DISABLED_OUTPUT}"

QUIET_OUTPUT="$(run_filtered tests/fixtures/alloc_in_loop.cpp \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang -quiet \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang -checks=alloc-in-loop)"
log_diagnostics "quiet alloc-in-loop" "${QUIET_OUTPUT}"
expect_output "${QUIET_OUTPUT}" "TCA001: C++ allocation expression inside loop"

NEGATIVE_OUTPUT="$(run_filtered tests/fixtures/no_alloc_in_loop.cpp)"
log_diagnostics "negative alloc fixture" "${NEGATIVE_OUTPUT}"
expect_no_diagnostics "${NEGATIVE_OUTPUT}"

NEGATIVE_MATH_OUTPUT="$(run_filtered tests/fixtures/no_math_in_loop.cpp \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang -checks=math-in-loop)"
log_diagnostics "negative math fixture" "${NEGATIVE_MATH_OUTPUT}"
expect_no_diagnostics "${NEGATIVE_MATH_OUTPUT}"

printf '\nall plugin tests passed\n'
