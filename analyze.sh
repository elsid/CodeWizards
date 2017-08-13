#!/usr/bin/env bash

set -ex

export CXX='ccache clang++'
mkdir -p cpp-cgdk-clang-analyze
cd cpp-cgdk-clang-analyze
cmake \
    -DCMAKE_CXX_FLAGS="--analyze" \
    ../cpp-cgdk
make clean
make -j$(nproc)
