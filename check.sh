#!/usr/bin/env bash

set -ex

./analyze.sh
./ubsan.sh
./asan.sh
./msan.sh
./kcov.sh

env ASAN_OPTIONS=strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1 \
    ./mult_run_test.py --binary cpp-cgdk-asan/bin/cpp-cgdk 1

env LD_LIBRARY_PATH=/home/elsid/dev/libcxx_msan/lib MSAN_OPTIONS=poison_in_dtor=1 \
    ./mult_run_test.py --binary cpp-cgdk-msan/bin/cpp-cgdk 1

./mult_run_test.py --binary cpp-cgdk-ubsan/bin/cpp-cgdk 1
