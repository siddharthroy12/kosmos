cmake_minimum_required(VERSION 3.1)
project(your-project-name-goes-here LANGUAGES C) # Change the name to your project here

set(CMAKE_C_STANDARD 99)

# Setup GLAD
add_library(glad libs/glad/glad.c)
target_include_directories(glad PUBLIC libs)

# Setup rayfork
# add_subdirectory(libs/rayfork)
add_library(rayfork libs/rayfork/rayfork.c)
target_include_directories(rayfork PUBLIC libs)

# Setup project
add_executable(${CMAKE_PROJECT_NAME} src/platform.c src/main.c)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE glad rayfork)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE libs/rayfork libs/sokol)
target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/") # This is useful to get an ASSETS_PATH in your IDE during development but you should comment this if you compile a release version and uncomment the next line
#target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC ASSETS_PATH="my-path-to-the-assets-directory") # Uncomment this line to setup the ASSETS_PATH macro to the final assets directory when you share the game

# Enable MacOS specific flags and link against it's specific libraries
if (APPLE)
    target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC "-framework Cocoa" "-framework QuartzCore" "-framework OpenGL")
    target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC "-fobjc-arc" "-ObjC")
endif()

# Enable Linux specific flags and link against it's specific libraries
if (UNIX)
    target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC m dl pthread X11 Xi Xcursor)
endif()
