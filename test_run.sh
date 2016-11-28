#!/usr/bin/env bash

set -ex

if ! [[ "${SEED}" ]]; then
    export SEED=$(python3 -c "import random; print(random.randint(0, 2**63))")
fi

VERBOSE_ID=$(date +%s)

env ID=${VERBOSE_ID} VERBOSE=1 FAIL_ON_EXCEPTION=1 ./run.sh

PROFILE_ID=$(date +%s)

env ID=${PROFILE_ID} CHECK_DURATION=1 PROFILE=1 FAIL_ON_EXCEPTION=1 ./run.sh

echo "verbose run log: log/${VERBOSE_ID}/run.log"
echo "profile run log: log/${PROFILE_ID}/run.log"

cat log/${VERBOSE_ID}/result.log
cat log/${PROFILE_ID}/result.log

./verbose_stats.py log/${VERBOSE_ID}/run.log | tee log/${VERBOSE_ID}/verbose_stats.log
./profile_stats.py log/${PROFILE_ID}/run.log | tee log/${PROFILE_ID}/profile_stats.log
