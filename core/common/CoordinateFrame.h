#pragma once

#include "Types.h"
#include "../geometry/Transform.h"
#include <string>

namespace cnc {

/**
 * @brief Generic coordinate frame abstraction
 * 
 * Represents a coordinate system with origin and axes. Provides utilities
 * for coordinate transformations and frame relationships.
 * 
 * Coordinate System Types in CNC:
 * 
 * 1. Machine Coordinates (MCS):
 *    - Absolute coordinate system fixed to the machine
 *    - Origin at machine home position (typically)
 *    - Axes aligned with machine axes (X, Y, Z)
 *    - Never changes during operation
 * 
 * 2. Workpiece Coordinates (WCS):
 *    - Coordinate system fixed to the workpiece
 *    - Origin at workpiece origin (corner, center, or custom)
 *    - Axes aligned with workpiece edges (for rectangular stock)
 *    - Defined by work offset (G54, G55, etc.)
 * 
 * 3. Tool Coordinates (TCS):
 *    - Coordinate system fixed to the tool
 *    - Origin at tool tip
 *    - Z-axis along tool centerline (up)
 *    - X/Y form horizontal plane
 * 
 * Transform Relationships:
 * - Machine -> Workpiece: Apply work offset inverse
 * - Workpiece -> Machine: Apply work offset
 * - Machine -> Tool: Apply tool holder transform
 * - Tool -> Machine: Apply tool holder inverse
 * 
 * G54-Style Offset Behavior:
 * - Work offset defines: workpiece_origin_in_machine_coords
 * - G-code coordinates are in workpiece frame
 * - Controller applies: machine_coords = workpiece_coords + work_offset
 * - Multiple work offsets allow multiple fixtures/parts
 */
class CoordinateFrame {
public:
    /**
     * @brief Construct coordinate frame
     * @param name Frame name/identifier
     * @param origin Origin position in parent frame
     * @param transform Transform from this frame to parent frame
     */
    CoordinateFrame(
        std::string name,
        const Vec3& origin = Vec3(0.0, 0.0, 0.0),
        const Transform& transform = Transform::identity()
    ) : name_(std::move(name)),
        origin_(origin),
        transform_(transform) {
    }

    /**
     * @brief Get frame name
     */
    const std::string& getName() const {
        return name_;
    }

    /**
     * @brief Get origin position
     */
    const Vec3& getOrigin() const {
        return origin_;
    }

    /**
     * @brief Set origin position
     */
    void setOrigin(const Vec3& origin) {
        origin_ = origin;
    }

    /**
     * @brief Get transform to parent frame
     */
    const Transform& getTransform() const {
        return transform_;
    }

    /**
     * @brief Set transform to parent frame
     */
    void setTransform(const Transform& transform) {
        transform_ = transform;
    }

    /**
     * @brief Transform point from this frame to parent frame
     */
    Vec3 toParent(const Vec3& point) const {
        return transform_.transformPoint(point);
    }

    /**
     * @brief Transform point from parent frame to this frame
     */
    Vec3 fromParent(const Vec3& point) const {
        Transform inverse = transform_.inverse();
        return inverse.transformPoint(point);
    }

    /**
     * @brief Get X-axis direction in parent frame
     */
    Vec3 getXAxis() const {
        return transform_.transformDirection(Vec3(1.0, 0.0, 0.0));
    }

    /**
     * @brief Get Y-axis direction in parent frame
     */
    Vec3 getYAxis() const {
        return transform_.transformDirection(Vec3(0.0, 1.0, 0.0));
    }

    /**
     * @brief Get Z-axis direction in parent frame
     */
    Vec3 getZAxis() const {
        return transform_.transformDirection(Vec3(0.0, 0.0, 1.0));
    }

    /**
     * @brief Check if frame is valid
     */
    bool isValid() const {
        return !name_.empty() &&
               std::isfinite(origin_.x) &&
               std::isfinite(origin_.y) &&
               std::isfinite(origin_.z);
    }

private:
    std::string name_;       ///< Frame name/identifier
    Vec3 origin_;            ///< Origin position in parent frame
    Transform transform_;     ///< Transform to parent frame
};

} // namespace cnc
