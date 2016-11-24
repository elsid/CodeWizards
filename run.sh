#!/usr/bin/env bash

set -ex

if ! [[ "${ID}" ]]; then
    ID=$(date +%s)
fi

if ! [[ "${RENDER_TO_SCREEN}" ]]; then
    RENDER_TO_SCREEN=false
fi

if ! [[ "${RENDER_TO_SCREEN_SYNC}" ]]; then
    RENDER_TO_SCREEN_SYNC=false
fi

mkdir -p log/
mkdir log/${ID}/

RUN_LOG=log/${ID}/run.log
GAME_LOG=log/${ID}/game.log
RESULT_LOG=log/${ID}/result.log
CONFIG=log/${ID}/local-runner.properties

cp local-runner.properties.tmpl ${CONFIG}

sed -i "s;{{ID}};${ID};g" ${CONFIG}
sed -i "s;{{RENDER_TO_SCREEN}};${RENDER_TO_SCREEN};g" ${CONFIG}
sed -i "s;{{RENDER_TO_SCREEN_SYNC}};${RENDER_TO_SCREEN_SYNC};g" ${CONFIG}
sed -i "s;{{SEED}};${SEED};g" ${CONFIG}

./run.py ${CONFIG} "${@}" | tee ${RUN_LOG}

echo "run log: ${RUN_LOG}"
