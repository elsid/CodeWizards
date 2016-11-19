#!/usr/bin/env bash

set -ex

java -Xms512m -Xmx2G -server -jar local-runner-ru/local-runner.jar local-runner.properties &
PID=${!}

cd python3-cgdk
sleep 2

LOG=log/run.$(date +%s).log

if [[ "${CPROFILE}" ]]; then
    python3 -m cProfile -s tottime Runner.py 2>&1 | tee ../${LOG}
else
    python3 Runner.py 2>&1 | tee ../${LOG}
fi

echo "log: ${LOG}"

kill ${PID}

wait
