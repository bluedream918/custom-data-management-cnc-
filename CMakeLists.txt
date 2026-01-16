cmake_minimum_required(VERSION 3.15)
project(CNC-IDE VERSION 1.0.0 LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(BUILD_IDE "Build the IDE application" ON)
option(BUILD_SIMULATOR "Build the simulator application" ON)
option(BUILD_TESTS "Build tests" ON)
option(BUILD_PLUGINS "Build plugin system" ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/render
)

# Core library (no UI dependencies)
add_subdirectory(core)

# Rendering engine
add_subdirectory(render)

# Rendering module (Qt OpenGL integration)
add_subdirectory(rendering)

# Plugins
if(BUILD_PLUGINS)
    add_subdirectory(plugins)
endif()

# Applications
if(BUILD_IDE)
    add_subdirectory(apps/ide)
endif()

if(BUILD_SIMULATOR)
    add_subdirectory(apps/simulator)
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
