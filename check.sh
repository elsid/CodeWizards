#!/usr/bin/env bash

set -ex

./analyze.sh
./ubsan.sh
./msan.sh
./asan.sh
./kcov.sh

./mult_run_test.py 1 cpp-cgdk-asan/bin/cpp-cgdk
env MSAN_OPTIONS=poison_in_dtor=1 ./mult_run_test.py 1 cpp-cgdk-msan/bin/cpp-cgdk
./mult_run_test.py 1 cpp-cgdk-ubsan/bin/cpp-cgdk
