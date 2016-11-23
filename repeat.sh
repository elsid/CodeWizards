#!/usr/bin/env bash

ID=$(date +%s)

mkdir -p log/
mkdir log/${ID}

LOG=log/${ID}/run.log

./repeat.py "${@}" | tee ${LOG}

echo "log: ${LOG}"
