#!/usr/bin/env bash

LOG=log/run.$(date +%s).log
./run.py | tee ${LOG}
echo "log: ${LOG}"
cat result.txt
