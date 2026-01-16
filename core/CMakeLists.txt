# Core library - ZERO UI or OpenGL dependencies
# This is the pure computational engine

add_library(cnc_core STATIC
    # Machine definitions & kinematics
    machine/machine.cpp
    machine/kinematics.cpp
    
    # Tool definitions
    tool/tool.cpp
    tool/tool_library.cpp
    
    # Material models
    material/material.cpp
    material/stock.cpp
    
    # Geometry utilities
    geometry/mesh.cpp
    geometry/voxel.cpp
    geometry/math_utils.cpp
    
    # CAM toolpath generation
    cam/toolpath.cpp
    cam/strategy.cpp
    
    # Process planning
    planner/planner.cpp
    planner/operation.cpp
    
    # Simulation engine
    simulation/simulator.cpp
    simulation/material_removal.cpp
    simulation/collision.cpp
    
    # G-code generation
    gcode/generator.cpp
    gcode/parser.cpp
    
    # Common utilities
    common/logger.cpp
    common/config.cpp
)

target_include_directories(cnc_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Core must remain UI-free
# No Qt, no OpenGL, no rendering dependencies
