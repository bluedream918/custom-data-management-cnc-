#pragma once

#include "ISimulationEngine.h"
#include "SimulationState.h"
#include "StepResult.h"
#include "../geometry/ToolSweep.h"
#include "../common/Error.h"
#include <memory>
#include <optional>

namespace cnc {

/**
 * @brief Controller for simulation step execution
 * 
 * Provides a high-level interface for executing simulation steps.
 * Engine-agnostic and UI-safe, can be called from:
 * - GUI event loops
 * - CLI batch processing
 * - RL environment loops
 * - Automated testing
 * 
 * Manages:
 * - Single step execution
 * - Multi-step execution
 * - State reset
 * - Result tracking
 */
class StepController {
public:
    /**
     * @brief Construct step controller
     * @param engine Simulation engine (takes ownership)
     */
    explicit StepController(std::unique_ptr<ISimulationEngine> engine)
        : engine_(std::move(engine)),
          lastResult_() {
        if (!engine_) {
            lastResult_ = StepResult::makeError(
                ErrorCode::InvalidArgument,
                "Engine cannot be null",
                false
            );
        }
    }

    /**
     * @brief Get simulation engine
     */
    ISimulationEngine* getEngine() {
        return engine_.get();
    }

    /**
     * @brief Get simulation engine (const)
     */
    const ISimulationEngine* getEngine() const {
        return engine_.get();
    }

    /**
     * @brief Initialize simulation state
     * 
     * Prepares the simulation state for execution.
     * 
     * @param state Simulation state to initialize
     * @return True if initialization succeeded
     */
    bool initialize(SimulationState& state) {
        if (!engine_) {
            lastResult_ = StepResult::makeError(
                ErrorCode::InvalidState,
                "Engine is null",
                false
            );
            return false;
        }

        Error error = engine_->initialize(state);
        if (error.isSuccess()) {
            lastResult_ = StepResult::success();
            return true;
        } else {
            lastResult_ = StepResult::makeError(
                error.getCode(),
                error.getMessage(),
                error.isRecoverable()
            );
            return false;
        }
    }

    /**
     * @brief Execute a single simulation step
     * 
     * Performs one step of simulation with the given tool sweep.
     * 
     * @param state Current simulation state
     * @param sweep Tool movement to simulate
     * @return True if step succeeded
     */
    bool stepOnce(SimulationState& state, const ToolSweep& sweep) {
        if (!engine_) {
            lastResult_ = StepResult::makeError(
                ErrorCode::InvalidState,
                "Engine is null",
                false
            );
            return false;
        }

        if (!state.isValid()) {
            lastResult_ = StepResult::makeError(
                ErrorCode::SimulationInvalidState,
                "Simulation state is invalid",
                false
            );
            return false;
        }

        lastResult_ = engine_->step(state, sweep);
        return lastResult_.isSuccess();
    }

    /**
     * @brief Execute N simulation steps
     * 
     * Performs multiple steps sequentially. Stops early if an error occurs.
     * 
     * @param state Current simulation state
     * @param sweep Tool movement to simulate (applied to each step)
     * @param numSteps Number of steps to execute
     * @return Number of steps successfully executed
     */
    std::uint64_t stepN(SimulationState& state, const ToolSweep& sweep, std::uint64_t numSteps) {
        if (!engine_ || !state.isValid()) {
            return 0;
        }

        std::uint64_t executed = 0;
        for (std::uint64_t i = 0; i < numSteps; ++i) {
            if (!stepOnce(state, sweep)) {
                break;
            }
            ++executed;
        }

        return executed;
    }

    /**
     * @brief Execute N simulation steps with different sweeps
     * 
     * Performs multiple steps with potentially different tool sweeps.
     * Useful for simulating toolpath segments.
     * 
     * @param state Current simulation state
     * @param sweeps Vector of tool sweeps (one per step)
     * @return Number of steps successfully executed
     */
    std::uint64_t stepN(SimulationState& state, const std::vector<ToolSweep>& sweeps) {
        if (!engine_ || !state.isValid() || sweeps.empty()) {
            return 0;
        }

        std::uint64_t executed = 0;
        for (const auto& sweep : sweeps) {
            if (!stepOnce(state, sweep)) {
                break;
            }
            ++executed;
        }

        return executed;
    }

    /**
     * @brief Reset simulation state
     * 
     * Resets the simulation state to initial conditions.
     * 
     * @param state Simulation state to reset
     * @return True if reset succeeded
     */
    bool reset(SimulationState& state) {
        if (!engine_) {
            lastResult_ = StepResult::makeError(
                ErrorCode::InvalidState,
                "Engine is null",
                false
            );
            return false;
        }

        Error error = engine_->reset(state);
        if (error.isSuccess()) {
            lastResult_ = StepResult::success();
            return true;
        } else {
            lastResult_ = StepResult::makeError(
                error.getCode(),
                error.getMessage(),
                error.isRecoverable()
            );
            return false;
        }
    }

    /**
     * @brief Get last step result
     * 
     * Returns the result of the most recent step operation.
     * 
     * @return Last step result, or nullopt if no step has been executed
     */
    const StepResult& getLastStepResult() const {
        return lastResult_;
    }

    /**
     * @brief Check if controller is valid
     */
    bool isValid() const {
        return engine_ != nullptr && engine_->isValid();
    }

    /**
     * @brief Check if last step was successful
     */
    bool lastStepSucceeded() const {
        return lastResult_.isSuccess();
    }

    /**
     * @brief Check if last step had a collision
     */
    bool lastStepHadCollision() const {
        return lastResult_.collisionDetected;
    }

    /**
     * @brief Check if last step had an error
     */
    bool lastStepHadError() const {
        return lastResult_.hasError();
    }

private:
    std::unique_ptr<ISimulationEngine> engine_;
    StepResult lastResult_;
};

} // namespace cnc
