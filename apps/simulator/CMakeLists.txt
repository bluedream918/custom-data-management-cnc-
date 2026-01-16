# Headless Simulator / CLI Runner
# No UI dependencies - pure simulation engine

add_executable(cnc_simulator
    main.cpp
    cli_runner.cpp
)

target_include_directories(cnc_simulator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link only against core (no render needed for headless)
target_link_libraries(cnc_simulator
    cnc_core
)
