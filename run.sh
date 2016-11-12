#!/usr/bin/env bash

set -ex

java -Xms512m -Xmx2G -server -jar local-runner-ru/local-runner.jar local-runner.properties &
PID=${!}

cd python3-cgdk
sleep 2
python3 -m cProfile -s tottime Runner.py 2>&1 | tee ../log/run.$(date +%s).log

kill ${PID}

wait
