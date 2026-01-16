#pragma once

#include "../material/MaterialGrid.h"
#include "../geometry/Transform.h"
#include "../common/Types.h"
#include <memory>
#include <cstdint>
#include <array>

namespace cnc {

/**
 * @brief Represents the full mutable state of a running simulation
 * 
 * Contains all state information needed to run and resume a CNC simulation.
 * Designed to be copyable for RL rollouts, checkpoints, and state snapshots.
 * 
 * This is the core state model that drives the simulation engine.
 */
class SimulationState {
public:
    /**
     * @brief Construct simulation state
     * @param materialGrid Initial material state (takes ownership)
     * @param initialToolPose Initial tool position and orientation
     */
    SimulationState(
        std::unique_ptr<MaterialGrid> materialGrid,
        const Transform& initialToolPose = Transform::identity()
    ) : materialGrid_(std::move(materialGrid)),
        toolPose_(initialToolPose),
        stepCount_(0),
        timeAccumulator_(0.0),
        deterministicSeed_(0) {
        // Initialize machine axes to zero
        machineAxes_.fill(0.0);
    }

    /**
     * @brief Copy constructor (deep copy)
     */
    SimulationState(const SimulationState& other)
        : toolPose_(other.toolPose_),
          machineAxes_(other.machineAxes_),
          stepCount_(other.stepCount_),
          timeAccumulator_(other.timeAccumulator_),
          deterministicSeed_(other.deterministicSeed_) {
        if (other.materialGrid_) {
            materialGrid_ = other.materialGrid_->clone();
        }
    }

    /**
     * @brief Copy assignment (deep copy)
     */
    SimulationState& operator=(const SimulationState& other) {
        if (this != &other) {
            toolPose_ = other.toolPose_;
            machineAxes_ = other.machineAxes_;
            stepCount_ = other.stepCount_;
            timeAccumulator_ = other.timeAccumulator_;
            deterministicSeed_ = other.deterministicSeed_;
            if (other.materialGrid_) {
                materialGrid_ = other.materialGrid_->clone();
            }
        }
        return *this;
    }

    /**
     * @brief Move constructor
     */
    SimulationState(SimulationState&&) noexcept = default;

    /**
     * @brief Move assignment
     */
    SimulationState& operator=(SimulationState&&) noexcept = default;

    /**
     * @brief Get current tool pose (position + orientation)
     */
    const Transform& getToolPose() const {
        return toolPose_;
    }

    /**
     * @brief Set tool pose
     */
    void setToolPose(const Transform& pose) {
        toolPose_ = pose;
    }

    /**
     * @brief Get current machine axis positions
     * 
     * Returns array of 6 values: [X, Y, Z, A, B, C]
     * Unused axes will be 0.0.
     */
    const std::array<double, 6>& getMachineAxes() const {
        return machineAxes_;
    }

    /**
     * @brief Set machine axis positions
     */
    void setMachineAxes(const std::array<double, 6>& axes) {
        machineAxes_ = axes;
    }

    /**
     * @brief Set a single axis position
     */
    void setAxis(Axis axis, double value) {
        machineAxes_[static_cast<int>(axis)] = value;
    }

    /**
     * @brief Get a single axis position
     */
    double getAxis(Axis axis) const {
        return machineAxes_[static_cast<int>(axis)];
    }

    /**
     * @brief Get material grid (mutable)
     */
    MaterialGrid* getMaterialGrid() {
        return materialGrid_.get();
    }

    /**
     * @brief Get material grid (const)
     */
    const MaterialGrid* getMaterialGrid() const {
        return materialGrid_.get();
    }

    /**
     * @brief Get step counter
     * 
     * Number of simulation steps executed so far.
     */
    std::uint64_t getStepCount() const {
        return stepCount_;
    }

    /**
     * @brief Increment step counter
     */
    void incrementStepCount() {
        ++stepCount_;
    }

    /**
     * @brief Get time accumulator
     * 
     * Total simulated time elapsed, in seconds.
     */
    double getTimeAccumulator() const {
        return timeAccumulator_;
    }

    /**
     * @brief Add time to accumulator
     */
    void addTime(double deltaTime) {
        timeAccumulator_ += deltaTime;
    }

    /**
     * @brief Get deterministic seed
     * 
     * Seed value for deterministic random number generation.
     * Used for RL reproducibility and debugging.
     */
    std::uint64_t getDeterministicSeed() const {
        return deterministicSeed_;
    }

    /**
     * @brief Set deterministic seed
     */
    void setDeterministicSeed(std::uint64_t seed) {
        deterministicSeed_ = seed;
    }

    /**
     * @brief Create a deep copy (snapshot) of this state
     * 
     * Useful for RL rollouts, checkpoints, and state history.
     */
    SimulationState clone() const {
        return SimulationState(*this);
    }

    /**
     * @brief Check if state is valid
     */
    bool isValid() const {
        return materialGrid_ != nullptr && materialGrid_->isValid();
    }

    /**
     * @brief Get remaining material volume
     */
    double getRemainingVolume() const {
        if (materialGrid_) {
            return materialGrid_->getRemainingVolume();
        }
        return 0.0;
    }

private:
    std::unique_ptr<MaterialGrid> materialGrid_;  ///< Material state
    Transform toolPose_;                           ///< Current tool pose
    std::array<double, 6> machineAxes_;           ///< Machine axis positions [X,Y,Z,A,B,C]
    std::uint64_t stepCount_ = 0;                 ///< Simulation step counter
    double timeAccumulator_ = 0.0;                ///< Total simulated time (seconds)
    std::uint64_t deterministicSeed_ = 0;         ///< Seed for deterministic RNG
};

} // namespace cnc
