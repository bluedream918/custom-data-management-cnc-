#pragma once

#include "Tool.h"
#include "../geometry/Transform.h"
#include "../common/Types.h"
#include <memory>

namespace cnc {

/**
 * @brief Represents how a tool is mounted in a machine
 * 
 * Encapsulates the relationship between a tool and its mounting
 * configuration. The tool holder defines the offset from the spindle
 * to the tool tip, enabling accurate tool tip pose calculation.
 * 
 * Coordinate system:
 * - Tool coordinate system: origin at tool tip, Z up
 * - Holder coordinate system: origin at spindle mount, Z down
 * - World coordinate system: machine coordinates
 * 
 * Industrial control assumptions:
 * - Holder length is fixed (rigid mounting)
 * - Tool is rigidly attached (no flex)
 * - Offset is deterministic and known
 */
class ToolHolder {
public:
    /**
     * @brief Construct tool holder
     * @param tool Tool reference (must remain valid)
     * @param holderLength Length from spindle mount to tool tip
     * @param holderOffset Offset from spindle center to tool center (typically 0,0,0)
     */
    ToolHolder(
        const Tool& tool,
        double holderLength,
        const Vec3& holderOffset = Vec3(0.0, 0.0, 0.0)
    ) : tool_(&tool),
        holderLength_(holderLength > 0.0 ? holderLength : 0.0),
        holderOffset_(holderOffset) {
    }

    /**
     * @brief Get tool reference
     */
    const Tool& getTool() const {
        return *tool_;
    }

    /**
     * @brief Get holder length
     * 
     * Distance from spindle mount point to tool tip.
     */
    double getHolderLength() const {
        return holderLength_;
    }

    /**
     * @brief Get holder offset
     * 
     * Offset from spindle center to tool center in holder coordinate system.
     * Typically (0,0,0) for collet holders, may be non-zero for special holders.
     */
    const Vec3& getHolderOffset() const {
        return holderOffset_;
    }

    /**
     * @brief Get total length from spindle to tool tip
     * 
     * Returns holder length plus tool length.
     */
    double getTotalLength() const {
        return holderLength_ + tool_->getTotalLength();
    }

    /**
     * @brief Compute tool tip pose in world coordinates
     * 
     * Calculates the tool tip pose given the spindle pose.
     * 
     * Transform chain:
     * 1. Spindle pose (machine kinematics)
     * 2. Apply holder offset
     * 3. Translate down by holder length + tool length
     * 
     * @param spindlePose Spindle pose in world coordinates
     * @return Tool tip pose in world coordinates
     */
    Transform computeToolTipPose(const Transform& spindlePose) const {
        // Start with spindle pose
        Transform toolPose = spindlePose;

        // Apply holder offset (typically zero, but allows for special holders)
        Vec3 offsetPos = toolPose.transformPoint(holderOffset_);
        toolPose = Transform(offsetPos, toolPose.getRotation());

        // Translate down by total length (tool tip is at bottom)
        // In machine coordinates, Z typically points up, so we subtract
        Vec3 downDirection = toolPose.transformDirection(Vec3(0.0, 0.0, -1.0));
        double totalLength = getTotalLength();
        Vec3 toolTipPos = toolPose.getPosition() + downDirection * totalLength;

        // Tool tip orientation is same as spindle (rigid mounting)
        return Transform(toolTipPos, toolPose.getRotation());
    }

    /**
     * @brief Get tool bounding box in world coordinates
     * 
     * Returns the bounding box of the tool in world coordinates,
     * given the spindle pose.
     * 
     * @param spindlePose Spindle pose in world coordinates
     * @return Tool bounding box in world coordinates
     */
    AABB getToolBoundingBox(const Transform& spindlePose) const {
        // Get tool bounding box in tool coordinate system
        AABB toolBounds = tool_->getBoundingBox();

        // Compute tool tip pose
        Transform toolTipPose = computeToolTipPose(spindlePose);

        // Transform tool bounds to world coordinates
        Vec3 minWorld = toolTipPose.transformPoint(toolBounds.min);
        Vec3 maxWorld = toolTipPose.transformPoint(toolBounds.max);

        // Compute union
        Vec3 worldMin(
            std::min(minWorld.x, maxWorld.x),
            std::min(minWorld.y, maxWorld.y),
            std::min(minWorld.z, maxWorld.z)
        );
        Vec3 worldMax(
            std::max(minWorld.x, maxWorld.x),
            std::max(minWorld.y, maxWorld.y),
            std::max(minWorld.z, maxWorld.z)
        );

        return AABB(worldMin, worldMax);
    }

    /**
     * @brief Check if holder is valid
     */
    bool isValid() const {
        return tool_ != nullptr &&
               tool_->isValid() &&
               holderLength_ > 0.0 &&
               std::isfinite(holderLength_);
    }

private:
    const Tool* tool_;       ///< Tool reference
    double holderLength_;     ///< Length from spindle to tool tip
    Vec3 holderOffset_;       ///< Offset from spindle center to tool center
};

} // namespace cnc
