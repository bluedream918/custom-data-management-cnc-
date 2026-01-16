# Rendering module - OpenGL viewport and rendering components
# This module provides Qt-OpenGL integration for the IDE

# Find Qt6 with OpenGL support
find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGLWidgets)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)

# Rendering library sources
set(RENDERING_SOURCES
    gl_viewport.cpp
)

set(RENDERING_HEADERS
    gl_viewport.h
)

# Create rendering library
add_library(cnc_rendering STATIC
    ${RENDERING_SOURCES}
    ${RENDERING_HEADERS}
)

target_include_directories(cnc_rendering PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link Qt OpenGL components
target_link_libraries(cnc_rendering
    Qt6::Core
    Qt6::Widgets
    Qt6::OpenGLWidgets
)
