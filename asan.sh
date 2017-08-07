#!/usr/bin/env bash

set -ex

export CXX='ccache clang++'
mkdir -p cpp-cgdk-asan
cd cpp-cgdk-asan
cmake \
    -DCMAKE_CXX_FLAGS="-g -O1 -fno-omit-frame-pointer -fsanitize=address -fsanitize-address-use-after-scope -fsanitize-recover=address -DELSID_INCREASED_TIMEOUT" \
    ../cpp-cgdk
make -j$(nproc)
env ASAN_OPTIONS=strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1 bin/cpp-cgdk-tests
