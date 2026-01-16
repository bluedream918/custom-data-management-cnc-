#pragma once

#include "../common/Error.h"
#include "../common/Types.h"
#include <cstdint>

namespace cnc {

/**
 * @brief Result of a single simulation step
 * 
 * Contains all information about the outcome of executing
 * one simulation step, including material removal, collisions,
 * and execution metrics.
 */
struct StepResult {
    /**
     * @brief Error code from the step execution
     */
    Error error = Error::success();

    /**
     * @brief Volume of material removed in this step
     * 
     * Measured in cubic units (mm³ or in³) matching the job units.
     */
    double materialRemovedVolume = 0.0;

    /**
     * @brief Flag indicating if a collision was detected
     */
    bool collisionDetected = false;

    /**
     * @brief Flag indicating if tool made contact with material
     */
    bool toolContact = false;

    /**
     * @brief Execution time delta for this step
     * 
     * Time elapsed during this simulation step, in seconds.
     * Used for performance metrics and time-based simulation.
     */
    double timeDelta = 0.0;

    /**
     * @brief Number of voxels/cells processed in this step
     * 
     * Useful for performance analysis and adaptive resolution.
     */
    std::uint64_t cellsProcessed = 0;

    /**
     * @brief Check if step was successful
     */
    bool isSuccess() const {
        return error.isSuccess() && !collisionDetected;
    }

    /**
     * @brief Check if step had an error
     */
    bool hasError() const {
        return error.isError();
    }

    /**
     * @brief Create a successful step result
     */
    static StepResult success(double timeDelta = 0.0) {
        StepResult result;
        result.timeDelta = timeDelta;
        return result;
    }

    /**
     * @brief Create an error step result
     */
    static StepResult makeError(ErrorCode code, const std::string& message, bool recoverable = false) {
        StepResult result;
        result.error = Error::make(code, message, recoverable);
        return result;
    }

    /**
     * @brief Create a collision step result
     */
    static StepResult collision(const std::string& message = "Collision detected") {
        StepResult result;
        result.collisionDetected = true;
        result.error = Error::make(ErrorCode::SimulationToolCollision, message, true);
        return result;
    }
};

} // namespace cnc
