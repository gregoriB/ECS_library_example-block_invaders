cmake_minimum_required(VERSION 3.16...3.31 FATAL_ERROR)

project(block_invaders LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.28.5
)

FetchContent_Declare(
    SDL2_image
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
    GIT_TAG release-2.6.3
)

FetchContent_Declare(
    SDL2_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG release-2.20.2
)

FetchContent_Declare(
    ECS
    GIT_REPOSITORY https://github.com/gregoriB/ECS_library.git
    GIT_TAG main
)

add_executable(game_run game/main.cpp)

FetchContent_MakeAvailable(SDL2)
FetchContent_MakeAvailable(SDL2_ttf)
FetchContent_MakeAvailable(ECS)

# TODO: Figure out why this isn't automatically defined like the other libraries
set(ECS_SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/ecs-src CACHE PATH "Path to ECS directory")

target_include_directories(game_run PRIVATE ${SDL2_SOURCE_DIR}/include)
target_include_directories(game_run PRIVATE ${SDL2_ttf_SOURCE_DIR}/include)
target_include_directories(game_run PRIVATE
    ${ECS_SOURCE_DIR}/include 
    $<BUILD_INTERFACE:${ECS_SOURCE_DIR}/include>
)

target_link_libraries(game_run PRIVATE
    SDL2::SDL2
    SDL2_ttf::SDL2_ttf
)

target_compile_features(game_run PUBLIC cxx_std_20)

target_compile_options(game_run PRIVATE
    -g
    -w
    -Decs_show_warnings
    -Decs_allow_debug
    -Decs_allow_unsafe
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Linking assets folder in build directory
add_custom_command(TARGET game_run POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_SOURCE_DIR}/game/assets
    ${CMAKE_BINARY_DIR}/assets
)
