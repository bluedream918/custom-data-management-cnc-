# GUI Library - Qt-based UI components
# This contains reusable UI components for the IDE

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)

# GUI depends on rendering module
# Note: This assumes rendering module is built before gui
# The actual executable will link both modules

# GUI library sources
set(GUI_SOURCES
    main_window.cpp
)

set(GUI_HEADERS
    main_window.h
)

# Create GUI library (or add to executable directly)
# For now, we'll add sources directly to the IDE executable
# This can be refactored into a library later if needed
