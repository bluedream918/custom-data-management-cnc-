#pragma once

#include "ISimulationEngine.h"
#include "../common/Time.h"
#include "../common/Error.h"
#include <string>

namespace cnc {

/**
 * @brief Base class for simulation engines implementing common step flow
 * 
 * Provides a foundation for concrete simulation engines by implementing
 * common functionality:
 * - Step counter management
 * - Time accumulation
 * - Error handling wrapper
 * - Deterministic behavior enforcement
 * 
 * Derived classes must implement:
 * - doStep() - actual cutting/material removal logic
 * - doInitialize() - engine-specific initialization
 * - doReset() - engine-specific reset logic
 */
class SimulationEngineBase : public ISimulationEngine {
public:
    /**
     * @brief Construct base engine
     * @param engineType Engine type identifier
     * @param fixedTimeStep Fixed time step for simulation (default: 0.001s)
     */
    explicit SimulationEngineBase(
        std::string engineType,
        double fixedTimeStep = 0.001
    ) : engineType_(std::move(engineType)),
        time_(fixedTimeStep),
        initialized_(false) {
    }

    virtual ~SimulationEngineBase() = default;

    /**
     * @brief Initialize simulation state
     * 
     * Wraps doInitialize() with common initialization logic.
     */
    Error initialize(SimulationState& state) override {
        if (!state.isValid()) {
            return Error::make(
                ErrorCode::SimulationInvalidState,
                "Simulation state is invalid",
                false
            );
        }

        Error result = doInitialize(state);
        if (result.isSuccess()) {
            initialized_ = true;
            time_.reset();
        }
        return result;
    }

    /**
     * @brief Execute a single simulation step
     * 
     * Wraps doStep() with common step flow:
     * - Validates state
     * - Updates step counter
     * - Accumulates time
     * - Calls derived doStep()
     * - Updates state with results
     */
    StepResult step(SimulationState& state, const ToolSweep& sweep) override {
        if (!initialized_) {
            return StepResult::makeError(
                ErrorCode::SimulationInvalidState,
                "Engine not initialized. Call initialize() first.",
                true
            );
        }

        if (!state.isValid()) {
            return StepResult::makeError(
                ErrorCode::SimulationInvalidState,
                "Simulation state is invalid",
                false
            );
        }

        // Update state step counter
        state.incrementStepCount();

        // Execute engine-specific step logic
        StepResult result = doStep(state, sweep);

        // Accumulate time in state
        state.addTime(time_.getTimeDelta());

        // Advance time for next step
        time_.step();

        return result;
    }

    /**
     * @brief Reset simulation state
     * 
     * Wraps doReset() with common reset logic.
     */
    Error reset(SimulationState& state) override {
        Error result = doReset(state);
        if (result.isSuccess()) {
            initialized_ = false;
            time_.reset();
        }
        return result;
    }

    /**
     * @brief Get engine type identifier
     */
    std::string getType() const override {
        return engineType_;
    }

    /**
     * @brief Check if engine is valid and ready
     */
    bool isValid() const override {
        return time_.isValid() && !engineType_.empty();
    }

    /**
     * @brief Check if engine is initialized
     */
    bool isInitialized() const {
        return initialized_;
    }

    /**
     * @brief Get time manager
     */
    const SimulationTime& getTime() const {
        return time_;
    }

    /**
     * @brief Get time manager (mutable)
     */
    SimulationTime& getTime() {
        return time_;
    }

protected:
    /**
     * @brief Engine-specific initialization
     * 
     * Derived classes implement this to perform engine-specific
     * initialization of the simulation state.
     * 
     * @param state Simulation state to initialize
     * @return Error code
     */
    virtual Error doInitialize(SimulationState& state) {
        // Default: no-op, success
        return Error::success();
    }

    /**
     * @brief Engine-specific step execution
     * 
     * Derived classes implement this to perform the actual
     * material removal and collision detection.
     * 
     * @param state Current simulation state
     * @param sweep Tool movement to simulate
     * @return Step result
     */
    virtual StepResult doStep(SimulationState& state, const ToolSweep& sweep) = 0;

    /**
     * @brief Engine-specific reset
     * 
     * Derived classes implement this to perform engine-specific
     * reset operations.
     * 
     * @param state Simulation state to reset
     * @return Error code
     */
    virtual Error doReset(SimulationState& state) {
        // Default: no-op, success
        return Error::success();
    }

private:
    std::string engineType_;
    SimulationTime time_;
    bool initialized_;
};

} // namespace cnc
