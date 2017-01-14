#!/usr/bin/env bash

set -ex

if [[ -d kcov-tests ]]; then
    rm -r kcov-tests
fi

mkdir -p cpp-cgdk-kcov
cd cpp-cgdk-kcov
cmake \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS='-DELSID_INCREASED_TIMEOUT -DELSID_STRATEGY_LOCAL' \
    ../cpp-cgdk
make -j$(nproc)

cd ..

kcov \
    --include-pattern="${PWD}/cpp-cgdk/" \
    --exclude-pattern="${PWD}/cpp-cgdk/tests/" \
    kcov-tests \
    cpp-cgdk-kcov/bin/cpp-cgdk-tests
