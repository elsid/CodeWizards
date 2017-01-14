#!/usr/bin/env bash

set -ex

mkdir -p cpp-cgdk-asan
cd cpp-cgdk-asan
cmake \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_CXX_FLAGS="-O1 -fsanitize=address -g -fno-omit-frame-pointer -DELSID_INCREASED_TIMEOUT" \
    ../cpp-cgdk
make -j$(nproc)
bin/cpp-cgdk-tests
