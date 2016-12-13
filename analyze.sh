#!/usr/bin/env bash

set -ex

mkdir -p cpp-cgdk-clang-analyze
cd cpp-cgdk-clang-analyze
cmake cmake \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_CXX_FLAGS="--analyze" \
    ../cpp-cgdk
make clean
make -j$(nproc)
