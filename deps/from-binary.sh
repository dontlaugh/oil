#!/usr/bin/env bash
#
# For things we don't want to compile.
#
# Usage:
#   deps/from-binary.sh <function name>
#
# Example:
#   deps/from-binary.sh download-clang
#   deps/from-binary.sh extract-clang

set -o nounset
set -o pipefail
set -o errexit

REPO_ROOT=$(cd "$(dirname $0)/.."; pwd)

source build/common.sh

readonly DEPS_DIR=$REPO_ROOT/../oil_DEPS

download-clang() {

  # download into $DEPS_DIR and not _cache because Dockerfile.clang stores the
  # compressed version

  wget --no-clobber --directory _cache \
    https://github.com/llvm/llvm-project/releases/download/llvmorg-14.0.0/clang+llvm-14.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz
}

extract-clang() {
  ### For developers

  pushd $DEPS_DIR
  time tar -x --xz < ../oil/_cache/clang+llvm-14.0.0*.tar.xz
  popd
}

extract-clang-in-container() {
  ### For Dockerfile.clang

  pushd $DEPS_DIR
  time tar -x --xz < clang+llvm-14.0.0*.tar.xz
  popd
}

test-clang() {
  $CLANGXX --version
}

"$@"
