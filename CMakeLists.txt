cmake_minimum_required(VERSION 3.0)
project(kosmos C)

set(CMAKE_C_STANDARD 99)

# Setting parameters for raylib
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # or games
add_subdirectory(libs/raylib)

file(GLOB_RECURSE APP_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.c)
add_executable(${CMAKE_PROJECT_NAME} ${APP_SOURCES})
target_link_libraries(${CMAKE_PROJECT_NAME} raylib)

target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC GAME_VERSION="1.0-DEV")

target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/") # Set the asset path macro to the absolute path on the dev machine
#target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC ASSETS_PATH="assets/") # Set the asset path macro in release more
