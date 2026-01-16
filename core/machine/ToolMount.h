#pragma once

#include "../tool/ToolHolder.h"
#include "../geometry/Transform.h"
#include "../common/Types.h"
#include <memory>
#include <optional>

namespace cnc {

/**
 * @brief Tool mounting interface for machines
 * 
 * Manages tool attachment and detachment on a machine. The machine
 * owns the tool mount, and tools are optional (machine may run empty).
 * 
 * Industrial control assumptions:
 * - Machine owns the mount (not shared)
 * - Tool is optional (nullptr = no tool mounted)
 * - Tool changes are atomic operations
 * - Tool tip pose is computed from machine kinematics + holder
 * 
 * Tool pose derivation:
 * 1. Machine kinematics provides spindle pose
 * 2. Tool holder provides offset from spindle to tool tip
 * 3. Tool tip pose = spindle pose + holder offset + tool length
 */
class ToolMount {
public:
    /**
     * @brief Construct empty tool mount
     */
    ToolMount() : holder_(nullptr) {
    }

    /**
     * @brief Attach tool holder
     * 
     * Mounts a tool holder in the machine. Replaces any existing tool.
     * 
     * @param holder Tool holder to attach (takes ownership)
     */
    void attachTool(std::unique_ptr<ToolHolder> holder) {
        if (holder && holder->isValid()) {
            holder_ = std::move(holder);
        }
    }

    /**
     * @brief Detach tool holder
     * 
     * Removes the current tool holder. Machine runs empty after this.
     */
    void detachTool() {
        holder_.reset();
    }

    /**
     * @brief Check if tool is mounted
     */
    bool hasTool() const {
        return holder_ != nullptr;
    }

    /**
     * @brief Get active tool holder
     * 
     * @return Pointer to tool holder, or nullptr if no tool mounted
     */
    const ToolHolder* getToolHolder() const {
        return holder_.get();
    }

    /**
     * @brief Get active tool
     * 
     * @return Pointer to tool, or nullptr if no tool mounted
     */
    const Tool* getTool() const {
        if (holder_) {
            return &holder_->getTool();
        }
        return nullptr;
    }

    /**
     * @brief Compute tool tip pose
     * 
     * Calculates the tool tip pose in world coordinates given the
     * spindle pose from machine kinematics.
     * 
     * If no tool is mounted, returns the spindle pose.
     * 
     * @param spindlePose Spindle pose from machine kinematics
     * @return Tool tip pose, or spindle pose if no tool mounted
     */
    Transform computeToolTipPose(const Transform& spindlePose) const {
        if (holder_) {
            return holder_->computeToolTipPose(spindlePose);
        }
        // No tool mounted - return spindle pose
        return spindlePose;
    }

    /**
     * @brief Get tool bounding box in world coordinates
     * 
     * Returns the bounding box of the mounted tool, or empty AABB
     * if no tool is mounted.
     * 
     * @param spindlePose Spindle pose from machine kinematics
     * @return Tool bounding box, or empty AABB if no tool
     */
    AABB getToolBoundingBox(const Transform& spindlePose) const {
        if (holder_) {
            return holder_->getToolBoundingBox(spindlePose);
        }
        // No tool - return empty bounding box
        return AABB(Vec3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, 0.0));
    }

    /**
     * @brief Check if mount is valid
     */
    bool isValid() const {
        if (holder_) {
            return holder_->isValid();
        }
        // Empty mount is valid
        return true;
    }

private:
    std::unique_ptr<ToolHolder> holder_;  ///< Mounted tool holder (nullptr if empty)
};

} // namespace cnc
