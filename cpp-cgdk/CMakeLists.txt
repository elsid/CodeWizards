cmake_minimum_required(VERSION 2.6)
project(cpp-cgdk)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_LINUX")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-optimize-sibling-calls")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wtype-limits")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/googletest/googletest/include)
include_directories(${CMAKE_SOURCE_DIR}/googletest/googlemock/include)

set(SOURCES
    csimplesocket/ActiveSocket.cpp
    csimplesocket/HTTPActiveSocket.cpp
    csimplesocket/PassiveSocket.cpp
    csimplesocket/SimpleSocket.cpp
    model/Bonus.cpp
    model/Building.cpp
    model/CircularUnit.cpp
    model/Game.cpp
    model/LivingUnit.cpp
    model/Message.cpp
    model/Minion.cpp
    model/Move.cpp
    model/Player.cpp
    model/PlayerContext.cpp
    model/Projectile.cpp
    model/Status.cpp
    model/Tree.cpp
    model/Unit.cpp
    model/Wizard.cpp
    model/World.cpp
    MyStrategy.cpp
    RemoteProcessClient.cpp
    Strategy.cpp

    russian-ai-cup-visual/Debug.cpp

    altmov.cpp
    bobyqa.cpp
    trsbox.cpp
    update.cpp

    action.cpp
    base_strategy.cpp
    debug_strategy.cpp
    time_limited_strategy.cpp
    optimal_position.cpp
    optimal_target.cpp
    optimal_destination.cpp
    optimal_movement.cpp
    optimal_path.cpp
    world_graph.cpp
    graph.cpp
    move_mode.cpp
    battle_mode.cpp
    circle.cpp
    helpers.cpp
    retreat_mode.cpp
    skills.cpp
    master_strategy.cpp
    stats.cpp
    move_to_node.cpp
    move_to_position.cpp
    simulation/simulation_strategy.cpp
)

add_executable(cpp-cgdk
    ${SOURCES}

    Runner.cpp
)

target_link_libraries(cpp-cgdk
    m
)

enable_testing()

add_subdirectory(googletest)

add_executable(cpp-cgdk-tests
    ${SOURCES}

    tests/action.cpp
    tests/main.cpp
    tests/math.cpp
    tests/my_strategy.cpp
    tests/optimal_destination.cpp
    tests/optimal_movement.cpp
    tests/optimal_path.cpp
    tests/optimal_position.cpp
    tests/optimal_target.cpp
)

target_link_libraries(cpp-cgdk-tests
    gmock
)
