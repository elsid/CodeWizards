#!/usr/bin/env bash

set -ex

export CXX='ccache clang++'
mkdir -p cpp-cgdk-ubsan
cd cpp-cgdk-ubsan
cmake \
    -DCMAKE_CXX_FLAGS="-fsanitize=undefined -g -fno-omit-frame-pointer -DELSID_INCREASED_TIMEOUT" \
    ../cpp-cgdk
make -j$(nproc)
bin/cpp-cgdk-tests
