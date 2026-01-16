#pragma once

#include <cstdint>
#include <limits>
#include <cmath>

namespace cnc {

/**
 * @brief Deterministic simulation time utilities
 * 
 * Provides time management for simulation without relying on OS timers.
 * All time calculations are deterministic and reproducible.
 */
class SimulationTime {
public:
    /**
     * @brief Construct with fixed time step
     * @param fixedTimeStep Fixed time step in seconds (must be > 0)
     */
    explicit SimulationTime(double fixedTimeStep = 0.001) 
        : fixedTimeStep_(fixedTimeStep > 0.0 ? fixedTimeStep : 0.001),
          accumulatedTime_(0.0),
          stepCount_(0) {
    }

    /**
     * @brief Get fixed time step
     */
    double getFixedTimeStep() const {
        return fixedTimeStep_;
    }

    /**
     * @brief Set fixed time step
     * @param timeStep New time step in seconds (must be > 0)
     */
    void setFixedTimeStep(double timeStep) {
        if (timeStep > 0.0) {
            fixedTimeStep_ = timeStep;
        }
    }

    /**
     * @brief Get accumulated simulation time
     */
    double getAccumulatedTime() const {
        return accumulatedTime_;
    }

    /**
     * @brief Get step count
     */
    std::uint64_t getStepCount() const {
        return stepCount_;
    }

    /**
     * @brief Advance time by one step
     * 
     * Adds fixedTimeStep to accumulated time and increments step count.
     */
    void step() {
        accumulatedTime_ += fixedTimeStep_;
        ++stepCount_;
    }

    /**
     * @brief Advance time by N steps
     * @param steps Number of steps to advance
     */
    void stepN(std::uint64_t steps) {
        accumulatedTime_ += fixedTimeStep_ * static_cast<double>(steps);
        stepCount_ += steps;
    }

    /**
     * @brief Get time delta for current step
     * 
     * Returns the fixed time step value.
     */
    double getTimeDelta() const {
        return fixedTimeStep_;
    }

    /**
     * @brief Reset time accumulator and step count
     */
    void reset() {
        accumulatedTime_ = 0.0;
        stepCount_ = 0;
    }

    /**
     * @brief Check if time is valid
     */
    bool isValid() const {
        return fixedTimeStep_ > 0.0 && 
               std::isfinite(accumulatedTime_) &&
               std::isfinite(fixedTimeStep_);
    }

    /**
     * @brief Calculate number of steps needed for a duration
     * @param duration Duration in seconds
     * @return Number of steps (rounded up)
     */
    std::uint64_t stepsForDuration(double duration) const {
        if (fixedTimeStep_ <= 0.0) {
            return 0;
        }
        return static_cast<std::uint64_t>(std::ceil(duration / fixedTimeStep_));
    }

    /**
     * @brief Calculate duration for a number of steps
     * @param steps Number of steps
     * @return Duration in seconds
     */
    double durationForSteps(std::uint64_t steps) const {
        return fixedTimeStep_ * static_cast<double>(steps);
    }

private:
    double fixedTimeStep_;           ///< Fixed time step in seconds
    double accumulatedTime_;         ///< Total accumulated time
    std::uint64_t stepCount_;        ///< Number of steps executed
};

/**
 * @brief Variable time step controller
 * 
 * Allows variable time steps while maintaining deterministic behavior.
 * Useful for adaptive simulation or variable-speed playback.
 */
class VariableTimeStep {
public:
    /**
     * @brief Construct with initial time step
     * @param initialTimeStep Initial time step in seconds
     */
    explicit VariableTimeStep(double initialTimeStep = 0.001)
        : currentTimeStep_(initialTimeStep > 0.0 ? initialTimeStep : 0.001),
          accumulatedTime_(0.0),
          stepCount_(0) {
    }

    /**
     * @brief Get current time step
     */
    double getCurrentTimeStep() const {
        return currentTimeStep_;
    }

    /**
     * @brief Set current time step
     * @param timeStep New time step in seconds (must be > 0)
     */
    void setCurrentTimeStep(double timeStep) {
        if (timeStep > 0.0) {
            currentTimeStep_ = timeStep;
        }
    }

    /**
     * @brief Get accumulated simulation time
     */
    double getAccumulatedTime() const {
        return accumulatedTime_;
    }

    /**
     * @brief Get step count
     */
    std::uint64_t getStepCount() const {
        return stepCount_;
    }

    /**
     * @brief Advance time by current time step
     */
    void step() {
        accumulatedTime_ += currentTimeStep_;
        ++stepCount_;
    }

    /**
     * @brief Advance time by a specific delta
     * @param deltaTime Time delta in seconds (must be > 0)
     */
    void stepBy(double deltaTime) {
        if (deltaTime > 0.0) {
            accumulatedTime_ += deltaTime;
            ++stepCount_;
        }
    }

    /**
     * @brief Get time delta for current step
     */
    double getTimeDelta() const {
        return currentTimeStep_;
    }

    /**
     * @brief Reset time accumulator and step count
     */
    void reset() {
        accumulatedTime_ = 0.0;
        stepCount_ = 0;
    }

    /**
     * @brief Check if time is valid
     */
    bool isValid() const {
        return currentTimeStep_ > 0.0 &&
               std::isfinite(accumulatedTime_) &&
               std::isfinite(currentTimeStep_);
    }

private:
    double currentTimeStep_;          ///< Current time step in seconds
    double accumulatedTime_;         ///< Total accumulated time
    std::uint64_t stepCount_;        ///< Number of steps executed
};

} // namespace cnc
