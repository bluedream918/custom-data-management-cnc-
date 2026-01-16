# Rendering engine - OpenGL/Vulkan abstraction
# Separated from core to maintain clean architecture

add_library(cnc_render STATIC
    renderer.cpp
    shader.cpp
    mesh_renderer.cpp
    viewport.cpp
)

target_include_directories(cnc_render PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link against core (one-way dependency: render -> core)
target_link_libraries(cnc_render
    cnc_core
)

# TODO: Add OpenGL/Vulkan dependencies when implementing
