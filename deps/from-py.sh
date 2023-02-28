#!/usr/bin/env bash
#
# Usage:
#   deps/from-py.sh <function name>

set -o nounset
set -o pipefail
set -o errexit

source build/dev-shell.sh  # may use different python3

dev-minimal() {
  # Python 2 packages for linting linting Python 2 code.
  pip install --user flake8 typing

  # Python 3 packages
  # - MyPy requires Python 3
  # - pexpect is for test/interactive.py
  pip3 install --user mypy pexpect
}

pea() {
  echo 'Not used by Dockerfile.pea'
  # pip3 install --user mypy
}

cpp() {
  # pexpect is for test/stateful
  python3 -m pip install --user mypy pexpect
}

"$@"
