#!/usr/bin/env bash

set -ex

FLAGS="-stdlib=libc++"
FLAGS="${FLAGS} -L/home/elsid/dev/libcxx_msan/lib"
FLAGS="${FLAGS} -I/home/elsid/dev/libcxx_msan/include"
FLAGS="${FLAGS} -I/home/elsid/dev/libcxx_msan/include/c++/v1"

mkdir -p cpp-cgdk-msan
cd cpp-cgdk-msan
cmake cmake \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_C_FLAGS="-O2 -fsanitize=memory -fsanitize-memory-track-origins -pthread -g -fno-omit-frame-pointer -fsanitize-memory-use-after-dtor ${FLAGS}" \
    -DCMAKE_CXX_FLAGS="-O2 -fsanitize=memory -fsanitize-memory-track-origins -pthread -g -fno-omit-frame-pointer -fsanitize-memory-use-after-dtor ${FLAGS}" \
    ../cpp-cgdk
make -j$(nproc)
env MSAN_OPTIONS=poison_in_dtor=1 bin/cpp-cgdk-tests
