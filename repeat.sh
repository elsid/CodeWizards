#!/usr/bin/env bash

LOG=log/run.$(date +%s).log
./repeat.py "${@}" | tee ${LOG}
echo "log: ${LOG}"
