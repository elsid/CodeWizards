#!/usr/bin/env bash

set -ex

mkdir -p cpp-cgdk-release
cd cpp-cgdk-release
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ ../cpp-cgdk
make -j$(nproc)
bin/cpp-cgdk-tests
