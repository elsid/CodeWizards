#!/usr/bin/env bash

set -ex

export LDFLAGS='--coverage'
export CXX='ccache g++'

mkdir -p cpp-cgdk-coverage
cd cpp-cgdk-coverage

cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS='-DELSID_STRATEGY_BASE --coverage' \
    ../cpp-cgdk
make -j$(nproc)

bin/cpp-cgdk-tests

rm -rf coverage
mkdir -p coverage

python2 /home/elsid/.local/bin/gcovr \
    --html \
    --html-details \
    --root="/home/elsid/dev/russianaicup/CodeWizards" \
    --filter='.*/cpp-cgdk/.*' \
    --exclude='.*(/(tests|googletest|model|csimplesocket)/)|((RemoteProcessClient|Runner|altmov|bobyqb|impl|prelim|rescue|trsbox|update|utils).*)' \
    --output='coverage/index.html'
