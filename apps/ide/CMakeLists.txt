# IDE Application - Qt-based UI
# This is ONLY UI/controller logic

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGLWidgets)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)

add_executable(cnc_ide
    main.cpp
    ${CMAKE_SOURCE_DIR}/gui/main_window.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/ProjectDock.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/PropertiesDock.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/ConsoleDock.cpp
    ${CMAKE_SOURCE_DIR}/src/render/PerspectiveCamera.cpp
    ${CMAKE_SOURCE_DIR}/src/render/Viewport3DWidget.cpp
    ${CMAKE_SOURCE_DIR}/src/render/GridRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/render/AxisRenderer.cpp
    project_manager.cpp
    viewport_widget.cpp
)

target_include_directories(cnc_ide PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/gui
    ${CMAKE_SOURCE_DIR}/rendering
    ${CMAKE_SOURCE_DIR}/src/ui
    ${CMAKE_SOURCE_DIR}/src/render
)

# Link against core, render, and rendering
target_link_libraries(cnc_ide
    cnc_core
    cnc_render
    cnc_rendering
    Qt6::Core
    Qt6::Widgets
    Qt6::OpenGLWidgets
)
