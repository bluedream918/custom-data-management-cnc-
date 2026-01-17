# CNC IDE + Simulation Platform

A professional, industrial-grade CNC IDE and simulation platform built with modern C++.

## Architecture Philosophy

This project follows a **strict separation of concerns** to ensure maintainability, testability, and scalability:

### Core Principles

1. **Core Independence**: The `core/` directory contains ZERO UI or rendering code. It is a pure computational engine that can be used in any context (CLI, GUI, web service, etc.).

2. **Unidirectional Dependencies**: 
   - `core/` → No dependencies (pure engine)
   - `render/` → Depends only on `core/`
   - `apps/ide/` → Depends on `core/` and `render/`
   - `apps/simulator/` → Depends only on `core/` (headless)

3. **Plugin Architecture**: Everything is designed to be extensible through plugins:
   - Machine definitions
   - Tool libraries
   - Postprocessors
   - Future: RL planners, custom strategies

4. **Offline-First**: No cloud dependencies. All computation happens locally.

5. **Cross-Platform**: Windows, macOS, Linux support through CMake and standard C++.

## Project Structure

```
cnc-ide/
├── apps/
│   ├── ide/                # Qt-based IDE application (UI only)
│   └── simulator/          # Headless simulator / CLI runner
│
├── core/                   # Core engine (NO UI, NO OpenGL)
│   ├── machine/            # Machine definitions & kinematics
│   ├── tool/               # Tool definitions
│   ├── material/           # Stock & material models
│   ├── geometry/           # Mesh, voxel, math utilities
│   ├── cam/                # Toolpath generation
│   ├── planner/            # Process planning
│   ├── simulation/         # Material removal, collision
│   ├── gcode/              # G-code generation
│   └── common/             # Logging, math, utilities
│
├── render/                 # Rendering engine (OpenGL/Vulkan abstraction)
│
├── plugins/                # Loadable extensions
│   ├── machines/
│   ├── tools/
│   └── postprocessors/
│
├── third_party/            # External dependencies (source or submodules)
│
├── data/                   # Runtime data
│   ├── machines/
│   ├── tools/
│   ├── materials/
│   └── examples/
│
├── tests/                  # Unit & integration tests
└── scripts/                # Build & dev scripts
```

## Build System

The project uses CMake for cross-platform builds.

### Prerequisites

- CMake 3.15 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- (Future) Qt 6 for IDE application
- (Future) OpenGL 3.3+ or Vulkan for rendering

### Building

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Build Options

- `BUILD_IDE`: Build the IDE application (default: ON)
- `BUILD_SIMULATOR`: Build the simulator application (default: ON)
- `BUILD_TESTS`: Build tests (default: ON)
- `BUILD_PLUGINS`: Build plugin system (default: ON)

## Development Guidelines

### Adding New Features

1. **Core Logic**: Add to `core/` - must remain UI-free
2. **Visualization**: Add to `render/` - rendering abstractions
3. **UI Integration**: Add to `apps/ide/` - Qt widgets and controllers
4. **Extensions**: Add to `plugins/` - loadable modules

### Code Organization

- Each module in `core/` should be self-contained
- Use forward declarations to minimize compile-time dependencies
- Keep interfaces minimal and focused
- Prefer composition over inheritance

### Testing

- Unit tests for `core/` modules (no UI dependencies)
- Integration tests for end-to-end workflows
- Render tests for visualization correctness

## Future Expansion

This architecture is designed to support:

- **Machine Types**: 3-axis, 4-axis, 5-axis, lathes, mills, routers
- **Tool Types**: End mills, drills, taps, custom geometries
- **CAM Strategies**: Adaptive clearing, trochoidal, high-speed machining
- **Planning**: Manual, semi-automatic, fully automatic with RL
- **Simulation**: Voxel-based, mesh-based, hybrid approaches
- **Postprocessors**: Fanuc, Haas, LinuxCNC, custom formats

## License

[To be determined]

## Contributing

[To be determined]
zip -e secure.zip file.txt

