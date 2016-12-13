#!/usr/bin/env bash

set -ex

if [[ -d kcov-tests ]]; then
    rm -r kcov-tests
fi

kcov \
    --include-pattern="${PWD}/cpp-cgdk/" \
    --exclude-pattern="${PWD}/cpp-cgdk/tests/" \
    kcov-tests \
    cpp-cgdk-debug/bin/cpp-cgdk-tests
