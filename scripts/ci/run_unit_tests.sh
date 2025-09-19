#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")"/../.. && pwd)"
TEST_ROOT="$ROOT_DIR/tests/unit"

if [[ ! -d "$TEST_ROOT" ]]; then
  echo "No unit tests found at $TEST_ROOT"
  exit 0
fi

shopt -s nullglob
suites=()
while IFS= read -r -d '' dir; do
  suites+=("$dir")
done < <(find "$TEST_ROOT" -mindepth 1 -maxdepth 2 -type f -name Makefile -print0 | xargs -0 -n1 dirname -z | sort -zu)

if [[ ${#suites[@]} -eq 0 ]]; then
  echo "No unit test suites discovered under $TEST_ROOT"
  exit 0
fi

failures=0
for suite in "${suites[@]}"; do
  echo "=============================="
  echo "Running suite: $suite"
  echo "=============================="
  ( cd "$suite" && make clean && make && make run ) || { 
    echo "Suite failed: $suite"; 
    failures=$((failures+1)); 
  }
  echo
done

if [[ $failures -ne 0 ]]; then
  echo "Unit test failures: $failures"
  exit 1
fi

echo "All unit tests passed."
