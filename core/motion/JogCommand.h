#pragma once

#include "../common/Types.h"
#include <cstdint>

namespace cnc {

/**
 * @brief Jog direction for manual movement
 */
enum class JogDirection {
    Positive,   ///< Move in positive direction
    Negative,   ///< Move in negative direction
    Stop        ///< Stop movement
};

/**
 * @brief Manual jog command (like pressing arrow keys)
 * 
 * Represents a manual movement command, similar to pressing
 * arrow keys on a CNC control panel. Used for manual tool
 * positioning and camera-follow motion.
 * 
 * Industrial control assumptions:
 * - Jog commands are velocity-based (not position-based)
 * - Speed is specified in units per second
 * - Duration or distance can be used to limit movement
 * - Commands are deterministic and repeatable
 */
class JogCommand {
public:
    /**
     * @brief Construct jog command by duration
     * @param axis Axis to move
     * @param direction Movement direction
     * @param speed Movement speed (units per second)
     * @param duration Duration in seconds (0 = continuous)
     */
    JogCommand(
        Axis axis,
        JogDirection direction,
        double speed,
        double duration = 0.0
    ) : axis_(axis),
        direction_(direction),
        speed_(speed > 0.0 ? speed : 0.0),
        duration_(duration >= 0.0 ? duration : 0.0),
        distance_(0.0),
        useDistance_(false) {
    }

    /**
     * @brief Construct jog command by distance
     * @param axis Axis to move
     * @param direction Movement direction
     * @param speed Movement speed (units per second)
     * @param distance Distance to move (units)
     */
    JogCommand(
        Axis axis,
        JogDirection direction,
        double speed,
        double distance,
        bool /* unused - for overload resolution */
    ) : axis_(axis),
        direction_(direction),
        speed_(speed > 0.0 ? speed : 0.0),
        duration_(0.0),
        distance_(distance > 0.0 ? distance : 0.0),
        useDistance_(true) {
    }

    /**
     * @brief Get target axis
     */
    Axis getAxis() const {
        return axis_;
    }

    /**
     * @brief Get movement direction
     */
    JogDirection getDirection() const {
        return direction_;
    }

    /**
     * @brief Get movement speed
     */
    double getSpeed() const {
        return speed_;
    }

    /**
     * @brief Get duration limit
     */
    double getDuration() const {
        return duration_;
    }

    /**
     * @brief Get distance limit
     */
    double getDistance() const {
        return distance_;
    }

    /**
     * @brief Check if using distance limit
     */
    bool isUsingDistance() const {
        return useDistance_;
    }

    /**
     * @brief Check if command is a stop command
     */
    bool isStop() const {
        return direction_ == JogDirection::Stop || speed_ <= 0.0;
    }

    /**
     * @brief Get target velocity
     * 
     * Returns the velocity that should be applied to the axis.
     * Positive for positive direction, negative for negative direction.
     */
    double getTargetVelocity() const {
        if (isStop()) {
            return 0.0;
        }

        double velocity = speed_;
        if (direction_ == JogDirection::Negative) {
            velocity = -velocity;
        }

        return velocity;
    }

    /**
     * @brief Check if command is valid
     */
    bool isValid() const {
        return speed_ >= 0.0 &&
               duration_ >= 0.0 &&
               distance_ >= 0.0 &&
               (useDistance_ || !useDistance_); // Always true, but explicit
    }

private:
    Axis axis_;
    JogDirection direction_;
    double speed_;
    double duration_;
    double distance_;
    bool useDistance_;
};

} // namespace cnc
