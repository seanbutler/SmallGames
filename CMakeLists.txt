cmake_minimum_required(VERSION 3.25)
project(SmallGames CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL3 REQUIRED CONFIG)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/app.cpp
    src/splash_screen.cpp
    src/menu_screen.cpp
    src/audio.cpp
    src/game.cpp
    src/renderer_utils.cpp
    src/pong/pong_game.cpp
    src/pong/pong_screen.cpp
    src/pong/ball.cpp
    src/pong/paddle.cpp
    src/breakout/breakout_game.cpp
    src/breakout/breakout_screen.cpp
    src/spaceinvaders/spaceinvaders_game.cpp
    src/spaceinvaders/spaceinvaders_screen.cpp
    src/asteroids/asteroids_game.cpp
    src/asteroids/asteroids_screen.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE src src/pong src/breakout src/spaceinvaders src/asteroids)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
