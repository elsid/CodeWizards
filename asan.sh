#!/usr/bin/env bash

set -ex

mkdir -p cpp-cgdk-asan
cd cpp-cgdk-asan
cmake cmake \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_CXX_FLAGS="-O1 -fsanitize=address -g -fno-omit-frame-pointer" \
    ../cpp-cgdk
make -j$(nproc)
bin/cpp-cgdk-tests