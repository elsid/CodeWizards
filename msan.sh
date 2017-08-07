#!/usr/bin/env bash

set -ex

export CC='ccache clang'
export CXX='ccache clang++'
FLAGS='-O2 -fsanitize=memory -fsanitize-memory-track-origins -pthread -g -fno-omit-frame-pointer -fsanitize-memory-use-after-dtor -L/home/elsid/dev/libcxx_msan/lib -I/home/elsid/dev/libcxx_msan/include -I/home/elsid/dev/libcxx_msan/include/c++/v1 -stdlib=libc++ -lc++abi -DELSID_INCREASED_TIMEOUT'

mkdir -p cpp-cgdk-msan
cd cpp-cgdk-msan
cmake \
    -DCMAKE_C_FLAGS="${FLAGS}" \
    -DCMAKE_CXX_FLAGS="${FLAGS}" \
    ../cpp-cgdk
make -j$(nproc)
env LD_LIBRARY_PATH=/home/elsid/dev/libcxx_msan/lib MSAN_OPTIONS=poison_in_dtor=1 bin/cpp-cgdk-tests
