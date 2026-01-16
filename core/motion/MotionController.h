#pragma once

#include "../machine/MachineKinematics.h"
#include "../machine/Axis.h"
#include <algorithm>
#include "JogCommand.h"
#include "../geometry/Transform.h"
#include "../common/Types.h"
#include <memory>
#include <array>
#include <vector>

namespace cnc {

/**
 * @brief Motion controller for executing jog and move commands
 * 
 * Manages axis state and executes motion commands deterministically.
 * Provides manual jogging (like arrow keys) and target position movement.
 * 
 * Industrial control assumptions:
 * - All motion is deterministic
 * - Axis limits are hard stops
 * - Velocity and acceleration limits are enforced
 * - No interpolation between commands (step-based updates)
 * - No threading (single-threaded execution)
 * 
 * This controller enables:
 * - Manual tool movement (jogging)
 * - Camera-follow motion
 * - CNC-like jogging behavior
 * - Deterministic simulation stepping
 */
class MotionController {
public:
    /**
     * @brief Construct motion controller
     * @param kinematics Machine kinematics (takes ownership)
     * @param maxVelocities Maximum velocities per axis [X,Y,Z,A,B,C] (units/sec)
     * @param maxAccelerations Maximum accelerations per axis [X,Y,Z,A,B,C] (units/sec²)
     */
    MotionController(
        std::unique_ptr<MachineKinematics> kinematics,
        const std::array<double, 6>& maxVelocities = {1000.0, 1000.0, 1000.0, 360.0, 360.0, 360.0},
        const std::array<double, 6>& maxAccelerations = {1000.0, 1000.0, 1000.0, 360.0, 360.0, 360.0}
    ) : kinematics_(std::move(kinematics)),
        axes_{} {
        // Initialize axes from kinematics limits
        auto limits = kinematics_->getAxisLimits();
        AxisConfig config = kinematics_->getAxisConfig();

        for (int i = 0; i < 6; ++i) {
            Axis axisType = static_cast<Axis>(i);
            if (config.hasAxis(axisType) && i < static_cast<int>(limits.size())) {
                axes_[i] = MachineAxis(
                    axisType,
                    limits[i].first,
                    limits[i].second,
                    maxVelocities[i],
                    maxAccelerations[i]
                );
            } else {
                // Initialize unused axes with default limits
                axes_[i] = MachineAxis(axisType, -1000.0, 1000.0, 0.0, 0.0);
            }
        }
    }

    /**
     * @brief Get machine kinematics
     */
    const MachineKinematics* getKinematics() const {
        return kinematics_.get();
    }

    /**
     * @brief Get axis by type
     */
    const MachineAxis& getAxis(Axis axisType) const {
        return axes_[static_cast<int>(axisType)];
    }

    /**
     * @brief Get axis by type (mutable)
     */
    MachineAxis& getAxis(Axis axisType) {
        return axes_[static_cast<int>(axisType)];
    }

    /**
     * @brief Get current tool pose
     * 
     * Calculates tool pose from current axis positions using forward kinematics.
     */
    Transform getCurrentToolPose() const {
        std::array<double, 6> positions;
        for (int i = 0; i < 6; ++i) {
            positions[i] = axes_[i].getPosition();
        }

        auto fkResult = kinematics_->forwardKinematics(positions);
        if (fkResult.valid) {
            return fkResult.toolPose;
        }

        // Fallback: identity transform if kinematics invalid
        return Transform::identity();
    }

    /**
     * @brief Get current axis positions
     */
    std::array<double, 6> getCurrentAxisPositions() const {
        std::array<double, 6> positions;
        for (int i = 0; i < 6; ++i) {
            positions[i] = axes_[i].getPosition();
        }
        return positions;
    }

    /**
     * @brief Apply jog command
     * 
     * Applies a jog command to the specified axis. The command
     * sets the target velocity for the axis, which will be
     * achieved gradually based on acceleration limits.
     * 
     * @param command Jog command to apply
     * @param deltaTime Time step for this update (seconds)
     */
    void applyJog(const JogCommand& command, double deltaTime) {
        if (!command.isValid() || deltaTime <= 0.0) {
            return;
        }

        if (command.isStop()) {
            // Stop the axis
            MachineAxis& axis = getAxis(command.getAxis());
            axis.update(0.0, deltaTime);
            return;
        }

        // Apply velocity command
        MachineAxis& axis = getAxis(command.getAxis());
        double targetVelocity = command.getTargetVelocity();

        // Handle distance-limited jog
        if (command.isUsingDistance()) {
            double remainingDistance = command.getDistance();
            double currentPos = axis.getPosition();
            double targetPos = currentPos + (targetVelocity > 0.0 ? remainingDistance : -remainingDistance);

            // Clamp target to limits
            targetPos = std::max(axis.getMinLimit(), std::min(axis.getMaxLimit(), targetPos));

            // Calculate distance to travel
            double distanceToTravel = std::abs(targetPos - currentPos);
            if (distanceToTravel < remainingDistance) {
                // Adjust velocity to stop at target
                double timeToStop = distanceToTravel / std::abs(targetVelocity);
                if (timeToStop < deltaTime) {
                    targetVelocity = (targetPos - currentPos) / deltaTime;
                }
            }
        }

        // Update axis with target velocity
        axis.update(targetVelocity, deltaTime);
    }

    /**
     * @brief Apply target position
     * 
     * Moves axes to achieve a target tool pose. Uses inverse kinematics
     * to calculate required axis positions, then moves axes toward
     * those positions.
     * 
     * This is a simplified implementation that directly sets axis
     * positions. A full implementation would include trajectory
     * planning and interpolation.
     * 
     * @param targetPose Target tool pose
     * @param deltaTime Time step for this update (seconds)
     * @return True if target was reached
     */
    bool applyTargetPosition(const Transform& targetPose, double deltaTime) {
        if (deltaTime <= 0.0) {
            return false;
        }

        // Calculate inverse kinematics
        auto ikSolutions = kinematics_->inverseKinematics(targetPose);
        if (ikSolutions.empty() || !ikSolutions[0].valid) {
            return false;
        }

        // Use first solution
        const auto& solution = ikSolutions[0];

        // Move each axis toward target position
        bool allReached = true;
        for (int i = 0; i < 6; ++i) {
            MachineAxis& axis = axes_[i];
            double targetPos = solution.axisPositions[i];
            double currentPos = axis.getPosition();
            double error = targetPos - currentPos;

            if (std::abs(error) < 1e-6) {
                // Already at target
                axis.update(0.0, deltaTime);
                continue;
            }

            // Calculate velocity needed to reach target
            // Simplified: use maximum velocity toward target
            double maxVel = axis.getMaxVelocity();
            double targetVel = (error > 0.0) ? maxVel : -maxVel;

            // Check if we'll overshoot
            double distanceToTravel = std::abs(targetVel * deltaTime);
            if (distanceToTravel > std::abs(error)) {
                // Adjust velocity to stop exactly at target
                targetVel = error / deltaTime;
            }

            axis.update(targetVel, deltaTime);

            // Check if reached
            double newError = targetPos - axis.getPosition();
            if (std::abs(newError) > 1e-6) {
                allReached = false;
            }
        }

        return allReached;
    }

    /**
     * @brief Update all axes (call this each simulation step)
     * 
     * Updates all axes with zero velocity (stops motion if no
     * command is active). Call this each simulation step to
     * ensure axes decelerate properly.
     * 
     * @param deltaTime Time step (seconds)
     */
    void update(double deltaTime) {
        if (deltaTime <= 0.0) {
            return;
        }

        // Update all axes (they will decelerate if no command active)
        for (auto& axis : axes_) {
            axis.update(0.0, deltaTime);
        }
    }

    /**
     * @brief Reset all axes to zero position
     */
    void reset() {
        for (auto& axis : axes_) {
            axis.reset();
        }
    }

    /**
     * @brief Check if controller is valid
     */
    bool isValid() const {
        if (!kinematics_ || !kinematics_->isValid()) {
            return false;
        }

        for (const auto& axis : axes_) {
            if (!axis.isValid()) {
                return false;
            }
        }

        return true;
    }

    /**
     * @brief Check if all axes are within limits
     */
    bool allAxesWithinLimits() const {
        for (const auto& axis : axes_) {
            if (!axis.isWithinLimits()) {
                return false;
            }
        }
        return true;
    }

private:
    std::unique_ptr<MachineKinematics> kinematics_;
    std::array<MachineAxis, 6> axes_;  ///< Axis states [X,Y,Z,A,B,C]
};

} // namespace cnc
