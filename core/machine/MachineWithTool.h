#pragma once

#include "MachineKinematics.h"
#include "ToolMount.h"
#include "../geometry/Transform.h"
#include "../common/Types.h"
#include <memory>
#include <array>
#include <vector>

namespace cnc {

/**
 * @brief Helper class combining machine kinematics and tool mount
 * 
 * Provides a convenient interface for machines that need both
 * kinematics and tool mounting capabilities. This is a composition
 * pattern (not inheritance) to avoid inheritance explosion.
 * 
 * Design principles:
 * - Clean separation: kinematics and tool mount are independent
 * - No inheritance explosion: uses composition, not multiple inheritance
 * - Tool-aware motion: provides tool tip pose directly
 * - Machine may run empty: tool is optional
 * 
 * Tool pose derivation:
 * 1. Machine kinematics: axis positions -> spindle pose
 * 2. Tool mount: spindle pose -> tool tip pose
 * 3. Final tool tip pose accounts for holder length and tool geometry
 */
class MachineWithTool {
public:
    /**
     * @brief Construct machine with tool mount
     * @param kinematics Machine kinematics (takes ownership)
     */
    explicit MachineWithTool(std::unique_ptr<MachineKinematics> kinematics)
        : kinematics_(std::move(kinematics)) {
    }

    /**
     * @brief Get machine kinematics
     */
    const MachineKinematics* getKinematics() const {
        return kinematics_.get();
    }

    /**
     * @brief Get tool mount
     */
    ToolMount& getToolMount() {
        return toolMount_;
    }

    /**
     * @brief Get tool mount (const)
     */
    const ToolMount& getToolMount() const {
        return toolMount_;
    }

    /**
     * @brief Check if tool is mounted
     */
    bool hasTool() const {
        return toolMount_.hasTool();
    }

    /**
     * @brief Get active tool
     */
    const Tool* getTool() const {
        return toolMount_.getTool();
    }

    /**
     * @brief Compute tool tip pose from axis positions
     * 
     * This is the main convenience method that combines kinematics
     * and tool mount to provide tool tip pose directly.
     * 
     * Process:
     * 1. Forward kinematics: axis positions -> spindle pose
     * 2. Tool mount: spindle pose -> tool tip pose
     * 
     * @param axisPositions Current axis positions [X,Y,Z,A,B,C]
     * @return Tool tip pose in machine coordinates, or spindle pose if no tool
     */
    Transform computeToolTipPose(const std::array<double, 6>& axisPositions) const {
        if (!kinematics_) {
            return Transform::identity();
        }

        // Forward kinematics: axis positions -> spindle pose
        auto fkResult = kinematics_->forwardKinematics(axisPositions);
        if (!fkResult.valid) {
            return Transform::identity();
        }

        // Tool mount: spindle pose -> tool tip pose
        return toolMount_.computeToolTipPose(fkResult.toolPose);
    }

    /**
     * @brief Compute inverse kinematics for tool tip pose
     * 
     * Calculates required axis positions to achieve a target tool tip pose.
     * 
     * Process:
     * 1. If tool mounted: tool tip pose -> spindle pose (accounting for holder)
     * 2. Inverse kinematics: spindle pose -> axis positions
     * 
     * @param targetToolTipPose Desired tool tip pose
     * @return Vector of inverse kinematics solutions
     */
    std::vector<InverseKinematicsResult> computeInverseKinematics(
        const Transform& targetToolTipPose) const {
        if (!kinematics_) {
            return {};
        }

        // If tool is mounted, we need to account for tool holder offset
        Transform targetSpindlePose = targetToolTipPose;
        
        if (toolMount_.hasTool()) {
            const ToolHolder* holder = toolMount_.getToolHolder();
            if (holder) {
                // Reverse the tool holder transform
                // Tool tip = spindle + holder offset + tool length
                // Spindle = tool tip - (holder offset + tool length)
                
                double totalLength = holder->getTotalLength();
                Vec3 holderOffset = holder->getHolderOffset();
                
                // Translate up by total length (reverse of tool mount transform)
                Vec3 upDirection = targetToolTipPose.transformDirection(Vec3(0.0, 0.0, 1.0));
                Vec3 spindlePos = targetToolTipPose.getPosition() - upDirection * totalLength;
                
                // Apply reverse holder offset
                Vec3 offsetPos = spindlePos - holderOffset;
                
                targetSpindlePose = Transform(offsetPos, targetToolTipPose.getRotation());
            }
        }

        // Inverse kinematics: spindle pose -> axis positions
        return kinematics_->inverseKinematics(targetSpindlePose);
    }

    /**
     * @brief Check if tool tip pose is reachable
     * 
     * Determines if a target tool tip pose can be achieved within
     * machine limits and constraints.
     * 
     * @param targetToolTipPose Tool tip pose to check
     * @return True if pose is reachable
     */
    bool isToolTipPoseReachable(const Transform& targetToolTipPose) const {
        auto solutions = computeInverseKinematics(targetToolTipPose);
        return !solutions.empty() && solutions[0].valid;
    }

    /**
     * @brief Get work envelope (from kinematics)
     */
    AABB getWorkEnvelope() const {
        if (kinematics_) {
            return kinematics_->getWorkEnvelope();
        }
        return AABB();
    }

    /**
     * @brief Get axis configuration (from kinematics)
     */
    AxisConfig getAxisConfig() const {
        if (kinematics_) {
            return kinematics_->getAxisConfig();
        }
        return AxisConfig();
    }

    /**
     * @brief Check if machine is valid
     */
    bool isValid() const {
        return kinematics_ != nullptr &&
               kinematics_->isValid() &&
               toolMount_.isValid();
    }

private:
    std::unique_ptr<MachineKinematics> kinematics_;
    ToolMount toolMount_;
};

} // namespace cnc
