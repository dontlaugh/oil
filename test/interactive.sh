#!/usr/bin/env bash
#
# Wrapper for test/interactive.py
#
# Usage:
#   test/interactive.sh <function name>

set -o nounset
set -o pipefail
set -o errexit

export PYTHONPATH=.

run() {
  ### Wrapper for PYTHONPATH

  test/interactive.py "$@"

}

all() {
  ### Run all tests

  # TODO: source build/dev-shell.sh to change $PATH?
  test/interactive.py bin/osh ../oil_DEPS/spec-bin/bash "$@"
}

soil-run() {
  ### Run it a few times to work around flakiness

  local n=5
  echo "Running $n times"

  local num_success=0
  local status=0

  for i in $(seq $n); do
    echo -----
    echo "Iteration $i"
    echo -----

    set +o errexit
    all
    status=$?
    set -o errexit

    if test "$status" -eq 0; then
      num_success=$((num_success + 1))
    fi
  done

  # This test is flaky, so only require 2 of 5 successes
  echo "test/interactive: $num_success of $n tries succeeded"

  if test "$num_success" -ge 2; then
    return 0
  else
    return 1
  fi
}

"$@"
