#!/usr/bin/env bash

set -ex

java -Xms512m -Xmx2G -server -jar local-runner-ru/local-runner.jar local-runner.properties &
PID=${!}

cd python3-cgdk
sleep 2

if [[ "${CPROFILE}" ]]; then
    python3 -m cProfile -s tottime Runner.py 2>&1 | tee ../log/run.$(date +%s).log
else
    python3 Runner.py 2>&1 | tee ../log/run.$(date +%s).log
fi

kill ${PID}

wait
