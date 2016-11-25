cmake_minimum_required(VERSION 2.6)
project(cpp-cgdk)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_LINUX")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-optimize-sibling-calls")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wtype-limits")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

include_directories(${CMAKE_SOURCE_DIR})

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

    newuoa.cpp
    optimal_position.cpp
    optimal_target.cpp
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

    tests/optimal_position.cpp
    tests/optimal_target.cpp
    tests/main.cpp
)

target_link_libraries(cpp-cgdk-tests
    gmock
)