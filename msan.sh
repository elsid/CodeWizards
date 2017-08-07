#!/usr/bin/env bash

set -ex

export CC='ccache clang'
export CXX='ccache clang++'
export LDFLAGS='-L/home/elsid/dev/libcxx_msan/lib -lc++abi -pthread'
COMPILER_FLAGS='-g -O2 -fno-omit-frame-pointer -fsanitize=memory -fsanitize-memory-track-origins -fsanitize-memory-use-after-dtor -I/home/elsid/dev/libcxx_msan/include -I/home/elsid/dev/libcxx_msan/include/c++/v1 -stdlib=libc++ -DELSID_INCREASED_TIMEOUT'

mkdir -p cpp-cgdk-msan
cd cpp-cgdk-msan
cmake \
    -DCMAKE_C_FLAGS="${COMPILER_FLAGS}" \
    -DCMAKE_CXX_FLAGS="${COMPILER_FLAGS}" \
    ../cpp-cgdk
make -j$(nproc)
env LD_LIBRARY_PATH=/home/elsid/dev/libcxx_msan/lib MSAN_OPTIONS=poison_in_dtor=1 bin/cpp-cgdk-tests
