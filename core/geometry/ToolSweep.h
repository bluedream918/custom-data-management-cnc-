#pragma once

#include "Transform.h"
#include "../tool/Tool.h"
#include "../common/Types.h"
#include <memory>
#include <algorithm>
#include <cmath>

namespace cnc {

/**
 * @brief Describes the swept volume of a tool movement
 * 
 * Represents the volume that a tool occupies as it moves from
 * one transform to another. Used by cutter engagement and
 * collision detection systems.
 */
class ToolSweep {
public:
    /**
     * @brief Construct tool sweep
     * @param tool Tool reference (must remain valid during sweep lifetime)
     * @param startTransform Starting tool pose
     * @param endTransform Ending tool pose
     * @param resolutionHint Sampling resolution hint for swept volume approximation
     */
    ToolSweep(
        const Tool& tool,
        const Transform& startTransform,
        const Transform& endTransform,
        double resolutionHint = 0.0
    ) : tool_(&tool),
        startTransform_(startTransform),
        endTransform_(endTransform),
        resolutionHint_(resolutionHint) {}

    /**
     * @brief Get tool reference
     */
    const Tool& getTool() const {
        return *tool_;
    }

    /**
     * @brief Get starting transform
     */
    const Transform& getStartTransform() const {
        return startTransform_;
    }

    /**
     * @brief Get ending transform
     */
    const Transform& getEndTransform() const {
        return endTransform_;
    }

    /**
     * @brief Get sampling resolution hint
     * 
     * Returns the suggested resolution for sampling the swept volume.
     * If 0.0, the system should use a default based on tool size.
     */
    double getResolutionHint() const {
        return resolutionHint_;
    }

    /**
     * @brief Get bounding box of the swept volume
     * 
     * Returns an AABB that encompasses the entire swept volume.
     * This is a conservative estimate for collision detection.
     */
    AABB getBoundingBox() const {
        // Get tool bounding box in tool coordinate system
        AABB toolBounds = tool_->getBoundingBox();
        
        // Transform tool bounds to start and end positions
        Vec3 startMin = startTransform_.transformPoint(toolBounds.min);
        Vec3 startMax = startTransform_.transformPoint(toolBounds.max);
        Vec3 endMin = endTransform_.transformPoint(toolBounds.min);
        Vec3 endMax = endTransform_.transformPoint(toolBounds.max);
        
        // Compute union of all corners
        Vec3 minCorner(
            std::min({startMin.x, startMax.x, endMin.x, endMax.x}),
            std::min({startMin.y, startMax.y, endMin.y, endMax.y}),
            std::min({startMin.z, startMax.z, endMin.z, endMax.z})
        );
        
        Vec3 maxCorner(
            std::max({startMin.x, startMax.x, endMin.x, endMax.x}),
            std::max({startMin.y, startMax.y, endMin.y, endMax.y}),
            std::max({startMin.z, startMax.z, endMin.z, endMax.z})
        );
        
        return AABB(minCorner, maxCorner);
    }

    /**
     * @brief Check if sweep is a pure translation (no rotation)
     */
    bool isTranslationOnly() const {
        const Quaternion& startRot = startTransform_.getRotation();
        const Quaternion& endRot = endTransform_.getRotation();
        
        // Check if rotations are approximately equal
        double dot = startRot.w * endRot.w + startRot.x * endRot.x + 
                    startRot.y * endRot.y + startRot.z * endRot.z;
        return std::abs(dot - 1.0) < 1e-6;
    }

    /**
     * @brief Get linear distance traveled
     */
    double getDistance() const {
        Vec3 delta = endTransform_.getPosition() - startTransform_.getPosition();
        return delta.length();
    }

    /**
     * @brief Get transform at a parameter t (0.0 = start, 1.0 = end)
     */
    Transform getTransformAt(double t) const {
        // Clamp t to [0, 1]
        t = std::max(0.0, std::min(1.0, t));
        
        // Interpolate position
        Vec3 startPos = startTransform_.getPosition();
        Vec3 endPos = endTransform_.getPosition();
        Vec3 pos = startPos + (endPos - startPos) * t;
        
        // Interpolate rotation (spherical linear interpolation)
        Quaternion startRot = startTransform_.getRotation();
        Quaternion endRot = endTransform_.getRotation();
        Quaternion rot = slerp(startRot, endRot, t);
        
        return Transform(pos, rot);
    }

private:
    /**
     * @brief Spherical linear interpolation between quaternions
     */
    static Quaternion slerp(const Quaternion& q1, const Quaternion& q2, double t) {
        double dot = q1.w * q2.w + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;
        
        // If dot < 0, negate one quaternion to take shorter path
        Quaternion q2Adjusted = q2;
        if (dot < 0.0) {
            q2Adjusted = Quaternion(-q2.w, -q2.x, -q2.y, -q2.z);
            dot = -dot;
        }
        
        // If quaternions are very close, use linear interpolation
        if (dot > 0.9995) {
            return Quaternion(
                q1.w + (q2Adjusted.w - q1.w) * t,
                q1.x + (q2Adjusted.x - q1.x) * t,
                q1.y + (q2Adjusted.y - q1.y) * t,
                q1.z + (q2Adjusted.z - q1.z) * t
            ).normalized();
        }
        
        // Spherical interpolation
        double theta = std::acos(dot);
        double sinTheta = std::sin(theta);
        double w1 = std::sin((1.0 - t) * theta) / sinTheta;
        double w2 = std::sin(t * theta) / sinTheta;
        
        return Quaternion(
            q1.w * w1 + q2Adjusted.w * w2,
            q1.x * w1 + q2Adjusted.x * w2,
            q1.y * w1 + q2Adjusted.y * w2,
            q1.z * w1 + q2Adjusted.z * w2
        ).normalized();
    }

    const Tool* tool_;
    Transform startTransform_;
    Transform endTransform_;
    double resolutionHint_;
};

} // namespace cnc
