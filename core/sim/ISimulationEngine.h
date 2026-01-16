#pragma once

#include "SimulationState.h"
#include "../geometry/ToolSweep.h"
#include "StepResult.h"
#include <memory>

namespace cnc {

/**
 * @brief Pure virtual interface for simulation engines
 * 
 * Defines the contract for any simulation engine implementation
 * (CNC milling, turning, additive, etc.). The engine is stateless
 * and operates on provided SimulationState objects.
 * 
 * This interface enables:
 * - Multiple simulation algorithms
 * - Plugin-based engines
 * - RL environment integration
 * - Testing with mock engines
 */
class ISimulationEngine {
public:
    virtual ~ISimulationEngine() = default;

    /**
     * @brief Initialize simulation state
     * 
     * Prepares the simulation state for execution. This may involve
     * validation, setup, or preprocessing of the material grid.
     * 
     * @param state Simulation state to initialize (modified in-place)
     * @return Error code indicating success or failure
     */
    virtual Error initialize(SimulationState& state) = 0;

    /**
     * @brief Execute a single simulation step
     * 
     * Performs one step of simulation, processing the tool sweep
     * and updating the simulation state accordingly.
     * 
     * The engine does NOT own the state - it operates on the provided
     * reference. This allows the caller to manage state lifecycle.
     * 
     * @param state Current simulation state (modified in-place)
     * @param sweep Tool movement to simulate
     * @return Step result with material removal, collisions, etc.
     */
    virtual StepResult step(SimulationState& state, const ToolSweep& sweep) = 0;

    /**
     * @brief Reset simulation state
     * 
     * Resets the simulation state to initial conditions. The exact
     * behavior depends on the engine implementation, but typically
     * involves resetting counters and restoring initial material state.
     * 
     * @param state Simulation state to reset (modified in-place)
     * @return Error code indicating success or failure
     */
    virtual Error reset(SimulationState& state) = 0;

    /**
     * @brief Create a deep copy of this engine
     * 
     * Returns a new engine instance with the same configuration.
     * Useful for parallel execution, RL rollouts, and state management.
     * 
     * @return Unique pointer to cloned engine
     */
    virtual std::unique_ptr<ISimulationEngine> clone() const = 0;

    /**
     * @brief Get engine type identifier
     * 
     * Returns a string identifying the engine type (e.g., "VoxelEngine",
     * "MeshEngine", "HybridEngine").
     */
    virtual std::string getType() const = 0;

    /**
     * @brief Check if engine is valid and ready
     * 
     * Returns true if the engine is properly configured and ready
     * to execute simulation steps.
     */
    virtual bool isValid() const = 0;
};

} // namespace cnc
