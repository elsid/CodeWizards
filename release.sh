#!/usr/bin/env bash

set -ex

VERSION=${1}
DIR=${PWD}/out/${VERSION}
ROOT=${PWD}

mkdir ${DIR}

cd cpp-cgdk

cp action.cpp ${DIR}
cp base_strategy.cpp ${DIR}
cp battle_mode.cpp ${DIR}
cp circle.cpp ${DIR}
cp graph.cpp ${DIR}
cp helpers.cpp ${DIR}
cp master_strategy.cpp ${DIR}
cp move_mode.cpp ${DIR}
cp move_to_node.cpp ${DIR}
cp MyStrategy.cpp ${DIR}
cp optimal_destination.cpp ${DIR}
cp optimal_movement.cpp ${DIR}
cp optimal_path.cpp ${DIR}
cp optimal_position.cpp ${DIR}
cp optimal_target.cpp ${DIR}
cp retreat_mode.cpp ${DIR}
cp skills.cpp ${DIR}
cp stats.cpp ${DIR}
cp time_limited_strategy.cpp ${DIR}
cp world_graph.cpp ${DIR}

cp action.hpp ${DIR}
cp base_strategy.hpp ${DIR}
cp battle_mode.hpp ${DIR}
cp cache.hpp ${DIR}
cp circle.hpp ${DIR}
cp common.hpp ${DIR}
cp context.hpp ${DIR}
cp golden_section.hpp ${DIR}
cp graph.hpp ${DIR}
cp helpers.hpp ${DIR}
cp line.hpp ${DIR}
cp master_strategy.hpp ${DIR}
cp math.hpp ${DIR}
cp minimize.hpp ${DIR}
cp mode.hpp ${DIR}
cp move_mode.hpp ${DIR}
cp move_to_node.hpp ${DIR}
cp MyStrategy.h ${DIR}
cp optimal_destination.hpp ${DIR}
cp optimal_movement.hpp ${DIR}
cp optimal_path.hpp ${DIR}
cp optimal_position.hpp ${DIR}
cp optimal_target.hpp ${DIR}
cp point.hpp ${DIR}
cp profiler.hpp ${DIR}
cp retreat_mode.hpp ${DIR}
cp skills.hpp ${DIR}
cp stats.hpp ${DIR}
cp target.hpp ${DIR}
cp time_limited_strategy.hpp ${DIR}
cp world_graph.hpp ${DIR}

cd ../bobyqa-cpp/

sed 's/<bobyqa.h>/"bobyqa.h"/' src/bobyqa.cpp > ${DIR}/bobyqa.cpp

cp include/bobyqa.h ${DIR}

cp src/altmov.cpp ${DIR}
cp src/altmov.hpp ${DIR}
cp src/bobyqb.hpp ${DIR}
cp src/impl.hpp ${DIR}
cp src/prelim.hpp ${DIR}
cp src/rescue.hpp ${DIR}
cp src/trsbox.cpp ${DIR}
cp src/trsbox.hpp ${DIR}
cp src/update.cpp ${DIR}
cp src/update.hpp ${DIR}
cp src/utils.hpp ${DIR}

cd ${DIR}

zip ../${VERSION}.zip *.hpp *.cpp *.h

ln -s ${ROOT}/cpp-cgdk-origin/Strategy.cpp Strategy.cpp
ln -s ${ROOT}/cpp-cgdk-origin/Strategy.h Strategy.h
ln -s ${ROOT}/cpp-cgdk-origin/Runner.cpp Runner.cpp
ln -s ${ROOT}/cpp-cgdk-origin/Runner.h Runner.h
ln -s ${ROOT}/cpp-cgdk-origin/RemoteProcessClient.cpp RemoteProcessClient.cpp
ln -s ${ROOT}/cpp-cgdk-origin/RemoteProcessClient.h RemoteProcessClient.h
ln -s ${ROOT}/cpp-cgdk-origin/model/ model
ln -s ${ROOT}/cpp-cgdk-origin/csimplesocket csimplesocket

echo ${DIR}/compilation.log

bash ${ROOT}/cpp-cgdk-origin/compile-g++14.sh || cat ${DIR}/compilation.log

echo 'done'
