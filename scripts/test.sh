#!/usr/bin/env bash
set -euo pipefail

./scripts/build.sh >/dev/null

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

expect_output() {
  local output="$1"
  local needle="$2"

  if ! grep -Fq "${needle}" <<<"${output}"; then
    printf 'missing expected output: %s\n' "${needle}" >&2
    printf '%s\n' "${output}" >&2
    exit 1
  fi
}

expect_no_warnings() {
  local output="$1"

  if grep -Fq 'warning:' <<<"${output}"; then
    printf 'expected no warnings, got:\n%s\n' "${output}" >&2
    exit 1
  fi
}

ALLOC_OUTPUT="$(run_plugin tests/fixtures/alloc_in_loop.cpp \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang -checks=alloc-in-loop)"

expect_output "${ALLOC_OUTPUT}" "TCA001: C++ allocation expression inside loop"
expect_output "${ALLOC_OUTPUT}" "TCA002: C++ deallocation expression inside loop"
expect_output "${ALLOC_OUTPUT}" "TCA003: C allocation call inside loop"
expect_output "${ALLOC_OUTPUT}" "TCA004: C deallocation call inside loop"
expect_output "${ALLOC_OUTPUT}" "consider reusing storage outside the loop"
expect_output "${ALLOC_OUTPUT}" "consider moving ownership cleanup outside the loop"
expect_output "${ALLOC_OUTPUT}" "consider allocating once before the loop"
expect_output "${ALLOC_OUTPUT}" "consider freeing loop-owned storage after the loop"

WARNING_COUNT="$(grep -c 'warning:' <<<"${ALLOC_OUTPUT}" || true)"
if [[ "${WARNING_COUNT}" -ne 6 ]]; then
  printf 'expected 6 warnings, got %s\n' "${WARNING_COUNT}" >&2
  printf '%s\n' "${ALLOC_OUTPUT}" >&2
  exit 1
fi

MATH_OUTPUT="$(run_plugin tests/fixtures/math_in_loop.cpp \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang -checks=math-in-loop)"

expect_output "${MATH_OUTPUT}" "TCA005: expensive math function call inside loop"
expect_output "${MATH_OUTPUT}" "consider hoisting loop-invariant math"

MATH_WARNING_COUNT="$(grep -c 'warning:' <<<"${MATH_OUTPUT}" || true)"
if [[ "${MATH_WARNING_COUNT}" -ne 4 ]]; then
  printf 'expected 4 math warnings, got %s\n' "${MATH_WARNING_COUNT}" >&2
  printf '%s\n' "${MATH_OUTPUT}" >&2
  exit 1
fi

DISABLED_OUTPUT="$(run_plugin tests/fixtures/alloc_in_loop.cpp \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang -checks=none)"
expect_no_warnings "${DISABLED_OUTPUT}"

QUIET_OUTPUT="$(run_plugin tests/fixtures/alloc_in_loop.cpp \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang -quiet \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang -checks=alloc-in-loop)"
expect_output "${QUIET_OUTPUT}" "TCA001: C++ allocation expression inside loop"

NEGATIVE_OUTPUT="$(run_plugin tests/fixtures/no_alloc_in_loop.cpp)"
expect_no_warnings "${NEGATIVE_OUTPUT}"

NEGATIVE_MATH_OUTPUT="$(run_plugin tests/fixtures/no_math_in_loop.cpp \
  -Xclang -plugin-arg-tensorium-clang-audit \
  -Xclang -checks=math-in-loop)"
expect_no_warnings "${NEGATIVE_MATH_OUTPUT}"

printf 'all plugin tests passed\n'
