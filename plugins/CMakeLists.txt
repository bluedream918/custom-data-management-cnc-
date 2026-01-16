# Plugin system
# Loadable extensions for machines, tools, postprocessors

# Plugin interface library
add_library(cnc_plugin_interface STATIC
    plugin_interface.cpp
)

target_include_directories(cnc_plugin_interface PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(cnc_plugin_interface
    cnc_core
)

# TODO: Add example plugins
# add_subdirectory(machines)
# add_subdirectory(tools)
# add_subdirectory(postprocessors)
