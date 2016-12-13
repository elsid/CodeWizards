#!/usr/bin/env bash

set -ex

mkdir -p cpp-cgdk-ubsan
cd cpp-cgdk-ubsan
cmake cmake \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_CXX_FLAGS="-fsanitize=undefined -g -fno-omit-frame-pointer" \
    ../cpp-cgdk
make -j$(nproc)
bin/cpp-cgdk-tests
