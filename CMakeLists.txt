cmake_minimum_required(VERSION 3.16...3.31 FATAL_ERROR)

project(block_invaders LANGUAGES CXX)

add_subdirectory(libraries/ecs)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

add_executable(game_run game/main.cpp)
target_compile_features(game_run INTERFACE cxx_std_20)
target_include_directories(game_run PRIVATE ${CMAKE_SOURCE_DIR}/libraries)
target_link_libraries(game_run SDL2::SDL2 SDL2_image::SDL2_image ECS::ecs)
target_compile_options(game_run INTERFACE
    -g
    -w
    -Decs_show_warnings
    -Decs_allow_debug
    -Decs_allow_unsafe
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
message(STATUS "GAME CMAKE RUNNING")
