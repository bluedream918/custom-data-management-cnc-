#pragma once

#include "AxisType.h"
#include "../common/Types.h"
#include <cmath>
#include <algorithm>

namespace cnc {

/**
 * @brief Represents a single CNC axis definition
 * 
 * Encapsulates axis configuration including travel limits, velocity,
 * acceleration, and resolution. This is a machine definition (not runtime state).
 * 
 * Industrial control assumptions:
 * - Position limits are in machine units (mm for linear, degrees for rotary)
 * - Velocity is in units per second
 * - Acceleration is in units per second squared
 * - Resolution is encoder step size (smallest position increment)
 * - All values are positive and finite
 */
class AxisDefinition {
public:
    /**
     * @brief Construct axis definition
     * @param type Axis type
     * @param minPosition Minimum travel position
     * @param maxPosition Maximum travel position
     * @param maxVelocity Maximum velocity (units/sec)
     * @param maxAcceleration Maximum acceleration (units/sec²)
     * @param resolution Encoder resolution (smallest step size)
     */
    AxisDefinition(
        AxisType type,
        double minPosition,
        double maxPosition,
        double maxVelocity,
        double maxAcceleration,
        double resolution = 0.001
    ) : type_(type),
        minPosition_(minPosition),
        maxPosition_(maxPosition),
        maxVelocity_(maxVelocity > 0.0 ? maxVelocity : 0.0),
        maxAcceleration_(maxAcceleration > 0.0 ? maxAcceleration : 0.0),
        resolution_(resolution > 0.0 ? resolution : 0.001) {
        // Ensure min < max
        if (minPosition_ > maxPosition_) {
            std::swap(minPosition_, maxPosition_);
        }
    }

    /**
     * @brief Get axis type
     */
    AxisType getType() const {
        return type_;
    }

    /**
     * @brief Get minimum position
     */
    double getMinPosition() const {
        return minPosition_;
    }

    /**
     * @brief Get maximum position
     */
    double getMaxPosition() const {
        return maxPosition_;
    }

    /**
     * @brief Get travel range
     */
    double getTravelRange() const {
        return maxPosition_ - minPosition_;
    }

    /**
     * @brief Get maximum velocity
     */
    double getMaxVelocity() const {
        return maxVelocity_;
    }

    /**
     * @brief Get maximum acceleration
     */
    double getMaxAcceleration() const {
        return maxAcceleration_;
    }

    /**
     * @brief Get encoder resolution
     */
    double getResolution() const {
        return resolution_;
    }

    /**
     * @brief Check if position is within limits
     */
    bool isPositionValid(double position) const {
        return position >= minPosition_ && position <= maxPosition_;
    }

    /**
     * @brief Clamp position to limits
     */
    double clampPosition(double position) const {
        return std::max(minPosition_, std::min(maxPosition_, position));
    }

    /**
     * @brief Check if axis is linear
     */
    bool isLinear() const {
        return isLinearAxis(type_);
    }

    /**
     * @brief Check if axis is rotary
     */
    bool isRotary() const {
        return isRotaryAxis(type_);
    }

    /**
     * @brief Check if axis definition is valid
     */
    bool isValid() const {
        return minPosition_ < maxPosition_ &&
               maxVelocity_ > 0.0 &&
               maxAcceleration_ > 0.0 &&
               resolution_ > 0.0 &&
               std::isfinite(minPosition_) &&
               std::isfinite(maxPosition_) &&
               std::isfinite(maxVelocity_) &&
               std::isfinite(maxAcceleration_) &&
               std::isfinite(resolution_);
    }

private:
    AxisType type_;           ///< Axis type
    double minPosition_;      ///< Minimum travel position
    double maxPosition_;      ///< Maximum travel position
    double maxVelocity_;      ///< Maximum velocity (units/sec)
    double maxAcceleration_; ///< Maximum acceleration (units/sec²)
    double resolution_;       ///< Encoder resolution (step size)
};

} // namespace cnc
