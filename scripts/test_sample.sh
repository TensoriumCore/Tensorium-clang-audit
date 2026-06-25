#!/usr/bin/env bash
set -euo pipefail

./scripts/build.sh >/dev/null

OUTPUT="$("./scripts/run_sample.sh" 2>&1)"

expect_output() {
  local needle="$1"

  if ! grep -Fq "${needle}" <<<"${OUTPUT}"; then
    printf 'missing expected output: %s\n' "${needle}" >&2
    printf '%s\n' "${OUTPUT}" >&2
    exit 1
  fi
}

expect_output "C allocation call inside loop"
expect_output "C deallocation call inside loop"
expect_output "dynamic allocation inside loop"
expect_output "dynamic deallocation inside loop"

WARNING_COUNT="$(grep -c 'warning:' <<<"${OUTPUT}" || true)"
if [[ "${WARNING_COUNT}" -ne 6 ]]; then
  printf 'expected 6 warnings, got %s\n' "${WARNING_COUNT}" >&2
  printf '%s\n' "${OUTPUT}" >&2
  exit 1
fi

printf 'sample plugin test passed\n'
